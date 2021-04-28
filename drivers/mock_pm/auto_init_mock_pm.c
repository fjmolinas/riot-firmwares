/*
 * Copyright (C) 2021 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     sys_auto_init_saul
 * @{
 * @file
 * @brief       Auto initialization for MOCK_PM dust sensor
 *
 * @author      Francisco Molina <francois-xavier.molina@inria.fr>
 * @}
 */

#include "assert.h"
#include "log.h"
#include "saul_reg.h"
#include "mock_pm.h"
#include "mock_pm_params.h"

/**
 * @brief   Allocate memory for the device descriptors
 */
static mock_pm_t mock_pm_devs[MOCK_PM_NUMOF];

#if IS_ACTIVE(MODULE_SAUL)
/**
 * @brief   Memory for the SAUL registry entries
 */
static saul_reg_t saul_entries[MOCK_PM_NUMOF * 2];

/**
 * @brief   Define the number of saul info
 */
#define MOCK_PM_INFO_NUM ARRAY_SIZE(mock_pm_saul_info)

/**
 * @name    Import SAUL endpoints
 * @{
 */
extern const saul_driver_t mock_pm_saul_driver_mc_pm_2p5;
extern const saul_driver_t mock_pm_saul_driver_mc_pm_10;
/** @} */
#endif

void auto_init_mock_pm(void)
{
#if IS_ACTIVE(MODULE_SAUL)
    assert(MOCK_PM_INFO_NUM == MOCK_PM_NUMOF);
#endif

    for (unsigned int i = 0; i < MOCK_PM_NUMOF; i++) {
        LOG_DEBUG("[auto_init_saul] initializing mock_pm #%u\n", i);
        mock_pm_init(&mock_pm_devs[i]);
#if IS_ACTIVE(MODULE_SAUL)
        saul_entries[(i * 2)].dev = &(mock_pm_devs[i]);
        saul_entries[(i * 2)].name = mock_pm_saul_info[i].name;
        saul_entries[(i * 2)].driver = &mock_pm_saul_driver_mc_pm_2p5;
        saul_entries[(i * 2) + 1].dev = &(mock_pm_devs[i]);
        saul_entries[(i * 2) + 1].name = mock_pm_saul_info[i].name;
        saul_entries[(i * 2) + 1].driver = &mock_pm_saul_driver_mc_pm_10;
        saul_reg_add(&(saul_entries[(i * 2)]));
        saul_reg_add(&(saul_entries[(i * 2) + 1]));
#endif
    }
}
