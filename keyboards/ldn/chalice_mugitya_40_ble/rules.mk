CUSTOM_MATRIX = lite
SRC += matrix.c
SRC += settings_sector.c
EXTRALDFLAGS += -Wl,--wrap=backing_store_unlock,--wrap=backing_store_erase
