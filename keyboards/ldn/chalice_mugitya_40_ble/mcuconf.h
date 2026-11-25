#pragma once
#include_next <mcuconf.h>

// The stock clock routine uses PLLM=25, PLLN=192, PLLP=2, PLLQ=4.
#undef STM32_PLLM_VALUE
#define STM32_PLLM_VALUE 25
#undef STM32_PLLN_VALUE
#define STM32_PLLN_VALUE 192
