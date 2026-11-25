# LDN Chalice Mugitya 40 BLE

This Vial-QMK port supports USB operation on the original STM32F411CEU6 board. Tests on one unit confirmed all 46 factory base-layer assignments, four configurable layers in Vial, and persistent key remapping after a USB power cycle. The firmware stores Vial settings in emulated EEPROM in sector 7 of the microcontroller’s internal flash.

## Build and update

From the Vial-QMK root, run `make ldn/chalice_mugitya_40_ble:vial`. The application is linked at `0x08020000`. Use the included `pack_ldn.py` to wrap the resulting binary in the `.Ldn` format accepted by the factory bootloader.

## Unlock in Vial

Follow the unlock prompt and hold the physical top-left and bottom-right keys together. The firmware checks their matrix positions, so the combination also works after either key has been remapped.
