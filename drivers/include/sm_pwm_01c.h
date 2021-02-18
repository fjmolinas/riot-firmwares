
/*
 * Copyright (C) 2021 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_sm_pwm_01c SM_PWM_01C dust sensor
 * @ingroup     drivers_sensors
 * @brief       Driver for Amphenol SM_PWM_01C infrared dust sensor
 * @{
 *
 * @file
 * @brief       SM_PWM_01C Device Driver
 *
 * @author      Francisco Molina <francois-xavier.molina@inria.fr>
 */

#ifndef SM_PWM_01C_H
#define SM_PWM_01C_H

#include <inttypes.h>

#include "timex.h"
#include "periph/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup    drivers_sm_pwm_01c_conf  SM_PWM_01C  compile configurations
 * @ingroup     drivers_sm_pwm_01c
 * @ingroup     config
 * @{
 */

/**
 * @def     CONFIG_SM_PWM_01C_SAMPLE_TIME
 *
 * @brief   Frequency at witch LPO % is calculated
 */
#ifndef CONFIG_SM_PWM_01C_SAMPLE_TIME
#define CONFIG_SM_PWM_01C_SAMPLE_TIME           (100*US_PER_MS)
#endif

/**
 * @def     CONFIG_SM_PWM_01C_WINDOW_TIME
 *
 * @brief   Length in time of the measuring window, recommended 5-30s
 */
#ifndef CONFIG_SM_PWM_01C_WINDOW_TIME
#define CONFIG_SM_PWM_01C_WINDOW_TIME     (10*US_PER_SEC)
#endif

#if defined(MODULE_SM_PWM_01C_MA) || defined(DOXYGEN)
/*
 * @brief   Length in time of the measuring window
 */
#define SM_PWM_01C_BUFFER_LEN    (CONFIG_SM_PWM_01C_WINDOW_TIME / \
                                  CONFIG_SM_PWM_01C_SAMPLE_TIME)
#else

/**
 * @def     CONFIG_SM_PWM_01C_EXP_WEIGHT
 *
 * @brief   Weight of the exponential average filter where:
 *          CONFIG_SM_PWM_01C_EXP_WEIGHT = 1 / (1 - alpha).
 *
 * @note    Should be chosen wisely, it can be done my minimizing MSE
 *          or other algorithms as Marquardt procedure.
 */
#ifndef CONFIG_SM_PWM_01C_EXP_WEIGHT
#define CONFIG_SM_PWM_01C_EXP_WEIGHT            (5)
#endif
#endif

/** @} */

#if defined(MODULE_SM_PWM_01C_MA) || defined(DOXYGEN)
/**
 * @brief   Circular buffer holding moving average values
 * @internal
 *
 */
typedef struct {
    uint16_t buf[SM_PWM_01C_BUFFER_LEN]; /**< circular buffer memory */
    size_t head;                         /**< current buffer head */
} circ_buf_t;
#endif

/**
 * @brief   Parameters for the SM_PWM_01c sensor
 *
 * These parameters are needed to configure the device at startup.
 */
typedef struct {
    gpio_t tsp_pin;                   /**< Low Pulse Signal Output (P1)
                                           of small Particle, active low */
    gpio_t tlp_pin;                   /**< Low Pulse Signal Output (P2)
                                           of large Particle, active low */
} sm_pwm_01c_params_t;

/**
 * @brief   LPO and concentration (ug/m3) values for small and large particles
 */
typedef struct {
    uint16_t mc_pm_2p5;         /**< Small particle concentration ug/m3 */
    uint16_t mc_pm_10;          /**< Large particle concentration ug/m3 */
} sm_pwm_01c_data_t;

/**
 * @brief   LPO and concentration (ug/m3) values for small and large particles
 * @internal
 */
typedef struct {
    uint32_t tsp_lpo;           /**< Small particle low Pulse active time us */
    uint32_t tlp_lpo;           /**< Large Particle low Pulse active time us */
#ifdef MODULE_SM_PWM_01C_MA
    circ_buf_t tsp_circ_buf;     /**< Small particle moving average values */
    circ_buf_t tlp_circ_buf;     /**< Large particle moving average values */
#else
    sm_pwm_01c_data_t data;
#endif
} sm_pwm_01c_values_t;

/**
 * @brief   Device descriptor for the SM_PWM_01c sensor
 */
typedef struct {
    sm_pwm_01c_params_t params;  /**< Device driver parameters */
    sm_pwm_01c_values_t values;  /**< Internal */
} sm_pwm_01c_t;


/**
 * @brief       Initialize the given SM_PWM_01C device
 *
 * @param[out]  dev         Initialized device descriptor of SM_PWM_01C device
 * @param[in]   params      The parameters for the SM_PWM_01C device
 *
 * @return                  0 on success
 *                         -EIO GPIO error
 */
int sm_pwm_01c_init(sm_pwm_01c_t* dev, const sm_pwm_01c_params_t* params);

/**
 * @brief       Start continuos measurement of Large and Small particle
 *              concentrations
 *
 * @param[in]   dev         Device descriptor of SM_PWM_01C device
 */
void sm_pwm_01c_start(sm_pwm_01c_t* dev);

/**
 * @brief       Stops continuos measurement of Large and Small particle
 *              concentration
 *
 * @param[in]   dev         Device descriptor of SM_PWM_01C device
 */
void sm_pwm_01c_stop(sm_pwm_01c_t* dev);

/**
 * @brief       Reads concentration values for small particle  (1 ~ 2um)
 *
 * @param[in]   dev        Device descriptor of SM_PWM_01C device
 * @param[out]  data       Pre-allocated memory to measured concentrations
 *
 */
void sm_pwm_01c_read_data(sm_pwm_01c_t *dev, sm_pwm_01c_data_t* data);

#ifdef __cplusplus
}
#endif

#endif /* SM_PWM_01C_PARAMS_H */
/** @} */
