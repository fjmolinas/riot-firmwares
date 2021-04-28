/*
 * Copyright (C) 2021 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_mock_pm MOCK_PM dust sensor
 * @ingroup     drivers_sensors
 * @brief       Mock particualate matter sensor
 * @{
 *
 * @file
 * @brief       MOCK_PM Device Driver
 *
 * @author      Francisco Molina <francois-xavier.molina@inria.fr>
 */

#ifndef MOCK_PM_H
#define MOCK_PM_H

#include <inttypes.h>

#include "timex.h"
#include "ztimer.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup    drivers_mock_pm_conf MOCK_PM compile configurations
 * @ingroup     drivers_mock_pm
 * @ingroup     config
 * @{
 */

/**
 * @def     CONFIG_MOCK_PM_SAMPLE_TIME
 *
 * @brief   Frequency at witch LPO % is calculated
 */
#ifndef CONFIG_MOCK_PM_SAMPLE_TIME
#define CONFIG_MOCK_PM_SAMPLE_TIME           (100 * US_PER_MS)
#endif

/**
 * @def     CONFIG_MOCK_PM_EXP_WEIGHT
 *
 * @brief   Weight of the exponential average filter where:
 *          CONFIG_MOCK_PM_EXP_WEIGHT = 1 / (1 - alpha).
 */
#ifndef CONFIG_MOCK_PM_EXP_WEIGHT
#define CONFIG_MOCK_PM_EXP_WEIGHT           (10)
#endif

/**
 * @def     CONFIG_MOCK_PM_10_MAX
 *
 */
#ifndef CONFIG_MOCK_PM_10_MAX
#define CONFIG_MOCK_PM_10_MAX               (100)
#endif

/**
 * @def     CONFIG_MOCK_PM_2P5_MAX
 *
 */
#ifndef CONFIG_MOCK_PM_2P5_MAX
#define CONFIG_MOCK_PM_2P5_MAX              (50)
#endif
/** @} */

/**
 * @brief  PM2.5 and PM10 mock values
 */
typedef struct {
    uint16_t mc_pm_2p5;         /**< PM2.5 ug/m3 */
    uint16_t mc_pm_10;          /**< PM10 ug/m3 */
} mock_pm_data_t;

/**
 * @brief   Moving
 */
typedef struct {
    mock_pm_data_t data;    /**< Current value for the exponentially averaged
                            particle concentration values */
    ztimer_t _sampler;      /**< internal sampling timer */
} mock_pm_t;

/**
 * @brief       Initialize the given MOCK_PM device
 *
 * @param[out]  dev         Initialized device descriptor of MOCK_PM device
 */
void mock_pm_init(mock_pm_t *dev);

/**
 * @brief       Reads particle concentration values
 *
 * @param[in]   dev        Device descriptor of MOCK_PM device
 * @param[out]  data       Pre-allocated memory to hold measured concentrations
 */
void mock_pm_read_data(mock_pm_t *dev, mock_pm_data_t *data);

#ifdef __cplusplus
}
#endif

#endif /* MOCK_PM_H */
/** @} */
