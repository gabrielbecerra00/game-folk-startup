/*
 * Copyright (c) 2023, Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ============ ti_msp_dl_config.h =============
 *  Configured MSPM0 DriverLib module declarations
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G350X
 *  by the SysConfig tool.
 */
#ifndef ti_msp_dl_config_h
#define ti_msp_dl_config_h

#define CONFIG_MSPM0G350X
#define CONFIG_MSPM0G3507

#if defined(__ti_version__) || defined(__TI_COMPILER_VERSION__)
#define SYSCONFIG_WEAK __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define SYSCONFIG_WEAK __weak
#elif defined(__GNUC__)
#define SYSCONFIG_WEAK __attribute__((weak))
#endif

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <ti/driverlib/m0p/dl_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform all required MSP DL initialization
 *
 *  This function should be called once at a point before any use of
 *  MSP DL.
 */


/* clang-format off */

#define POWER_STARTUP_DELAY                                                (16)


#define CPUCLK_FREQ                                                     32000000



/* Defines for TIMER_0 */
#define TIMER_0_INST                                                     (TIMA0)
#define TIMER_0_INST_IRQHandler                                 TIMA0_IRQHandler
#define TIMER_0_INST_INT_IRQN                                   (TIMA0_INT_IRQn)
#define TIMER_0_INST_LOAD_VALUE                                         (63999U)



/* Defines for SPI_0 */
#define SPI_0_INST                                                         SPI1
#define SPI_0_INST_IRQHandler                                   SPI1_IRQHandler
#define SPI_0_INST_INT_IRQN                                       SPI1_INT_IRQn
#define GPIO_SPI_0_PICO_PORT                                              GPIOB
#define GPIO_SPI_0_PICO_PIN                                       DL_GPIO_PIN_8
#define GPIO_SPI_0_IOMUX_PICO                                   (IOMUX_PINCM25)
#define GPIO_SPI_0_IOMUX_PICO_FUNC                   IOMUX_PINCM25_PF_SPI1_PICO
#define GPIO_SPI_0_POCI_PORT                                              GPIOB
#define GPIO_SPI_0_POCI_PIN                                      DL_GPIO_PIN_14
#define GPIO_SPI_0_IOMUX_POCI                                   (IOMUX_PINCM31)
#define GPIO_SPI_0_IOMUX_POCI_FUNC                   IOMUX_PINCM31_PF_SPI1_POCI
/* GPIO configuration for SPI_0 */
#define GPIO_SPI_0_SCLK_PORT                                              GPIOB
#define GPIO_SPI_0_SCLK_PIN                                       DL_GPIO_PIN_9
#define GPIO_SPI_0_IOMUX_SCLK                                   (IOMUX_PINCM26)
#define GPIO_SPI_0_IOMUX_SCLK_FUNC                   IOMUX_PINCM26_PF_SPI1_SCLK



/* Port definition for Pin Group BUZZER */
#define BUZZER_PORT                                                      (GPIOA)

/* Defines for PIN: GPIOA.24 with pinCMx 54 on package pin 25 */
#define BUZZER_PIN_PIN                                          (DL_GPIO_PIN_24)
#define BUZZER_PIN_IOMUX                                         (IOMUX_PINCM54)
/* Port definition for Pin Group OLED_CTRL */
#define OLED_CTRL_PORT                                                   (GPIOB)

/* Defines for CS: GPIOB.3 with pinCMx 16 on package pin 51 */
#define OLED_CTRL_CS_PIN                                         (DL_GPIO_PIN_3)
#define OLED_CTRL_CS_IOMUX                                       (IOMUX_PINCM16)
/* Defines for DC: GPIOB.6 with pinCMx 23 on package pin 58 */
#define OLED_CTRL_DC_PIN                                         (DL_GPIO_PIN_6)
#define OLED_CTRL_DC_IOMUX                                       (IOMUX_PINCM23)
/* Defines for RST: GPIOB.7 with pinCMx 24 on package pin 59 */
#define OLED_CTRL_RST_PIN                                        (DL_GPIO_PIN_7)
#define OLED_CTRL_RST_IOMUX                                      (IOMUX_PINCM24)
/* Port definition for Pin Group BUTTONS */
#define BUTTONS_PORT                                                     (GPIOA)

/* Defines for BTN1: GPIOA.12 with pinCMx 34 on package pin 5 */
#define BUTTONS_BTN1_PIN                                        (DL_GPIO_PIN_12)
#define BUTTONS_BTN1_IOMUX                                       (IOMUX_PINCM34)
/* Defines for BTN2: GPIOA.13 with pinCMx 35 on package pin 6 */
#define BUTTONS_BTN2_PIN                                        (DL_GPIO_PIN_13)
#define BUTTONS_BTN2_IOMUX                                       (IOMUX_PINCM35)
/* Defines for BTN3: GPIOA.14 with pinCMx 36 on package pin 7 */
#define BUTTONS_BTN3_PIN                                        (DL_GPIO_PIN_14)
#define BUTTONS_BTN3_IOMUX                                       (IOMUX_PINCM36)
/* Defines for BTN4: GPIOA.15 with pinCMx 37 on package pin 8 */
#define BUTTONS_BTN4_PIN                                        (DL_GPIO_PIN_15)
#define BUTTONS_BTN4_IOMUX                                       (IOMUX_PINCM37)
/* Defines for BTN5: GPIOA.16 with pinCMx 38 on package pin 9 */
#define BUTTONS_BTN5_PIN                                        (DL_GPIO_PIN_16)
#define BUTTONS_BTN5_IOMUX                                       (IOMUX_PINCM38)
/* Defines for BTN6: GPIOA.17 with pinCMx 39 on package pin 10 */
#define BUTTONS_BTN6_PIN                                        (DL_GPIO_PIN_17)
#define BUTTONS_BTN6_IOMUX                                       (IOMUX_PINCM39)
/* Defines for BTN7: GPIOA.23 with pinCMx 53 on package pin 24 */
#define BUTTONS_BTN7_PIN                                        (DL_GPIO_PIN_23)
#define BUTTONS_BTN7_IOMUX                                       (IOMUX_PINCM53)
/* Defines for BTN8: GPIOA.18 with pinCMx 40 on package pin 11 */
#define BUTTONS_BTN8_PIN                                        (DL_GPIO_PIN_18)
#define BUTTONS_BTN8_IOMUX                                       (IOMUX_PINCM40)


/* clang-format on */

void SYSCFG_DL_init(void);
void SYSCFG_DL_initPower(void);
void SYSCFG_DL_GPIO_init(void);
void SYSCFG_DL_SYSCTL_init(void);
void SYSCFG_DL_TIMER_0_init(void);
void SYSCFG_DL_SPI_0_init(void);


bool SYSCFG_DL_saveConfiguration(void);
bool SYSCFG_DL_restoreConfiguration(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_msp_dl_config_h */
