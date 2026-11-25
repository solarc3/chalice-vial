# Vial over USB for the original Chalice Mugitya 40 BLE

This project documents the key matrix and firmware update format of the original LDN Chalice Mugitya 40 BLE and provides a Vial-QMK port for wired use. Tests on one keyboard confirmed that all 46 base-layer assignments matched the factory profile, Vial recognized four layers, and a remapped key retained its setting after a USB power cycle. Version 0.0.2 uses the original LDN bootloader for firmware updates. Bluetooth and the factory Fn functions are outside the scope of this release.

## Hardware test results

| Test | Observation |
| --- | --- |
| Firmware installation | LDN Dual Mode Keyboard Tools installed the Vial package through the factory bootloader. After restarting, Linux identified the device as `Chalice Mugitya 40 USB Vial` (`FEED:C140`). |
| Vial discovery | The keyboard reported Vial protocol 6. Its 328-byte compressed layout definition decoded to the JSON used for the build. Vial displayed all four layers directly from the keyboard. |
| Base-layer mapping | All 46 keycodes read from the keyboard matched the factory profile. A physical press of the right arrow registered at matrix row 3, column 11 during testing of the initial QMK port. |
| Settings persistence | The layer 3 right-arrow position was assigned F13. After a USB power cycle, Linux enumerated the keyboard again (device 9 to 10). At 69 seconds of firmware uptime, the position still reported F13. A final read confirmed restoration of its original transparent assignment (`KC_TRNS`). The base-layer right arrow retained its original mapping throughout the test. |

[Vial desktop screenshot from the test unit](docs/vial-escritorio.png)

## Hardware and firmware analysis

The PCB carries the markings “Chalice Mugitya” and “Powered By LDN” and uses an STM32F411CEU6 microcontroller. The factory firmware identified itself over USB HID as `0483:2020` in keyboard mode and `0483:2021` in bootloader mode. [PCB photograph](docs/pcb.png) · [Microcontroller marking](docs/stm32.png)

The factory profile `Chalice_Mugitya_40_BLE.DMPhyCfg` supplied the matrix pins, the matrix position of each key, and its physical dimensions. The file `function_layer_0.Fl_Cfg` supplied the base-layer USB keycodes. The port reproduces the factory scan method by driving one row high at a time and reading the columns with their internal pull-downs enabled.

The LDN Windows utility ran under Wine/Proton on Linux. Its **Refresh list** action detected the physical keyboard as **Chalice[USB BL]**. The utility rejected a raw QMK binary and accepted an application wrapped in the LDN update format. Analysis of its file handling provided the basis for the packager described below.

The initial QMK build used the factory application address, `0x08020000`, and stored settings in RAM. It booted through the LDN loader and supported reading, remapping, and restoring keys through VIA. Version 0.0.2 uses the same scanner, embeds the layout definition for Vial, and stores settings in internal flash. To unlock protected settings in Vial, follow the unlock prompt and hold the physical top-left and bottom-right keys (End and Right Arrow on the base layer).

## Matrix and storage

| Resource | Configuration |
| --- | --- |
| Row pins, in scan order | PA3, PC15, PC14, PC13 |
| Column pins, in scan order | PB10, PB2, PB1, PB0, PB12, PB13, PB14, PB15, PA8, PA9, PA15, PB3, PB4 |
| Application | Internal flash sector 5, starting at `0x08020000`, with a 128 KiB limit |
| Vial settings | Internal flash sector 7, starting at `0x08060000`. A 16 KiB write log backs 8 KiB of emulated EEPROM. |
| Identifiers | Development USB VID/PID `FEED:C140` and Vial UID `B6 BE 0A 91 68 18 37 ED` |

On first use, the firmware checks that sector 7 is blank before writing an ownership marker at the end. On subsequent boots it recognizes the marker. Compacting the write log requires erasing the entire sector. The external SPI NOR flash remains available to the LDN system. [Sector handling](keyboards/ldn/chalice_mugitya_40_ble/settings_sector.c) · [STM32F411 reference manual](https://www.st.com/resource/en/reference_manual/dm00119316.pdf)

## LDN update format

The tested format consists of a 32-byte header followed by the application encoded in 32-bit words. Multibyte integers are little-endian. Header offsets are measured in bytes.

| Offset | Field |
| --- | --- |
| 0 | Format version |
| 2 | Device family |
| 4, 5 | Type markers |
| 6 | CRC-8 of the encoded payload, polynomial `0x31`, initial value zero |
| 7 | Package version |
| 9 | Application length in bytes |

The packager checks that the reset vector points into the Chalice application image. Repackaging the reference firmware reproduced the 76,728-byte factory file `ldn_dm_fw_V0.1.6.7.Ldn` from the April 2022 distribution byte for byte (SHA-256 `893b9a837232fafec3e68b264c568a62611a0ce866432622631ab2a4f80b341f`). Decoding the new QMK package also recovered the compiled binary exactly. [LDN packager](keyboards/ldn/chalice_mugitya_40_ble/pack_ldn.py)

## Build from source

Run these commands from the root of this repository with Git, Python 3, and the QMK ARM build environment installed. The checkout pins the Vial-QMK revision used for the tested build. [Vial porting guide](https://get.vial.today/docs/porting-to-vial.html)

```sh
git clone https://github.com/vial-kb/vial-qmk.git
git -C vial-qmk checkout dd43959ae5c08d8a28d38a1acf7b04e86b14a344
git -C vial-qmk submodule update --init --recursive
mkdir -p vial-qmk/keyboards/ldn
cp -R keyboards/ldn/chalice_mugitya_40_ble vial-qmk/keyboards/ldn/
cd vial-qmk
make SKIP_GIT=yes ldn/chalice_mugitya_40_ble:vial
python3 keyboards/ldn/chalice_mugitya_40_ble/pack_ldn.py \
  ldn_chalice_mugitya_40_ble_vial.bin Chalice-Vial-USB-0.0.2.Ldn --version 2
```

The packaged firmware is 42,292 bytes. Its SHA-256 is `4b0c6e5662760fb581874f52b896b676f0a171c82562054817e109bec1cc7b09`. [Version 0.0.2 firmware package](firmware/Chalice-Vial-USB-0.0.2.Ldn)

## Installation and recovery

1. Switch off the battery and disconnect USB. Hold the physical top-left key while reconnecting the cable.
2. Confirm the bootloader enumerates as `0483:2021`. In LDN Dual Mode Keyboard Tools V1.1, select **Chalice[USB BL]**.
3. Select the `.Ldn` package and start the update. When the transfer finishes, confirm the restart. Keep the keyboard powered until it reappears over USB.

Keep the original firmware package for your unit for recovery. [LDN Chalice guide](https://blog.csdn.net/lswhome/article/details/123768793) · [Manufacturer’s update and recovery procedure](https://blog.csdn.net/lswhome/article/details/115428099)
