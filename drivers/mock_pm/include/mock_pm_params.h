/*
 * Copyright (C) 2021 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_mock_pm
 * @{
 * @file
 * @brief       Default configuration for MOCK_PM driver
 *
 * @author      Francisco Molina <francois-xavier.molina@inria.fr>
 *
 */

#ifndef MOCK_PM_PARAMS_H
#define MOCK_PM_PARAMS_H

#include "board.h"
#include "saul_reg.h"
#include "mock_pm.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Set default configuration parameters for the MOCK_PM
 * @{
 */
#ifndef MOCK_PM_SAUL_INFO
#define MOCK_PM_SAUL_INFO             { .name = "mock-pm" }
#endif
/**@}*/

/**
 * @brief   The number of configured sensors
 */
#define MOCK_PM_NUMOF               (1)

/**
 * @brief   Additional meta information to keep in the SAUL registry
 */
static const saul_reg_info_t mock_pm_saul_info[] =
{
    MOCK_PM_SAUL_INFO
};

#ifdef __cplusplus
}
#endif

#endif /* MOCK_PM_PARAMS_H */
/** @} */
