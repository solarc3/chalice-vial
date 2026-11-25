"""Package a flash-linked Chalice application for LDN Dual Mode Keyboard Tools."""

import argparse
import struct
from pathlib import Path


APP_BASE = 0x08020000
APP_SIZE = 128 * 1024
# Generator format, device family, two type markers, CRC-8, version, byte count.
HEADER = struct.Struct("<HHBBBHI19x")
WORD_MASK = 0xF60CC72B


def crc8(data):
    crc = 0
    for byte in data:
        crc ^= byte
        for _ in range(8):
            crc = ((crc << 1) & 0xFF) ^ (0x31 if crc & 0x80 else 0)
    return crc


def package(image, version):
    if not 8 <= len(image) <= APP_SIZE or len(image) % 4:
        raise ValueError("expected a word-aligned application of at most 128 KiB")
    stack, reset = struct.unpack_from("<II", image)
    if not 0x20000000 < stack <= 0x20020000 or stack % 8:
        raise ValueError("initial stack pointer is outside aligned STM32F411 SRAM")
    if not (reset & 1 and APP_BASE <= (reset & ~1) < APP_BASE + len(image)):
        raise ValueError("expected a Thumb reset vector in the flash application")

    payload = bytearray(len(image))
    for offset in range(0, len(image), 4):
        (word,) = struct.unpack_from("<I", image, offset)
        rotated = (word >> 6) | ((word & 0x3F) << 26)
        struct.pack_into("<I", payload, offset, rotated ^ WORD_MASK)

    header = HEADER.pack(0x0100, 0x2021, 0xFE, 0xFF, crc8(payload), version, len(image))
    return header + payload


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("image", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--version", type=lambda value: int(value, 0), default=1)
    args = parser.parse_args()
    if not 0 <= args.version <= 0xFFFF:
        parser.error("version must fit in 16 bits")
    if args.image.resolve() == args.output.resolve() or args.output.suffix.lower() != ".ldn":
        parser.error("choose a separate .Ldn output file")
    try:
        encoded = package(args.image.read_bytes(), args.version)
        args.output.write_bytes(encoded)
    except (OSError, ValueError) as error:
        parser.exit(1, f"{error}\n")
