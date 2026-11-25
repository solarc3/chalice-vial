// SPDX-License-Identifier: GPL-2.0-or-later
#include "gpio.h"
#include "matrix.h"
#include "wait.h"

static const pin_t row_pins[MATRIX_ROWS] = MATRIX_ROW_PINS;
static const pin_t col_pins[MATRIX_COLS] = MATRIX_COL_PINS;

void matrix_init_custom(void) {
    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
        gpio_set_pin_input_low(col_pins[col]);
    }
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        gpio_set_pin_input(row_pins[row]);
        gpio_write_pin_high(row_pins[row]);
    }
}

bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    bool changed = false;
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        // The LDN scanner drives the selected row high and samples high columns.
        // Leave the other rows in high impedance during the scan.
        gpio_set_pin_output(row_pins[row]);
        wait_us(MATRIX_IO_DELAY);
        matrix_row_t pressed = 0;
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            if (gpio_read_pin(col_pins[col])) {
                pressed |= (matrix_row_t)1 << col;
            }
        }
        gpio_set_pin_input(row_pins[row]);
        matrix_output_unselect_delay(row, pressed != 0);
        if (current_matrix[row] != pressed) {
            current_matrix[row] = pressed;
            changed = true;
        }
    }
    return changed;
}
