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
 * @brief       SAUL adaption of the mock_pm dust sensor driver
 *
 * @author      Francisco Molina <francois-xavier.molina@inria.fr>
 * @}
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "phydat.h"
#include "saul.h"
#include "mock_pm.h"

static int read_mc_pm_2p5(const void *_dev, phydat_t *data)
{
    mock_pm_data_t values;
    mock_pm_t *dev = (mock_pm_t *)_dev;

    mock_pm_read_data(dev, &values);
    data->unit = UNIT_GPM3;
    data->scale = -6;
    uint32_t value = values.mc_pm_2p5;
    phydat_fit(data, (int32_t *)&value, 1);
    return 1;
}

static int read_mc_pm_10(const void *_dev, phydat_t *data)
{
    mock_pm_data_t values;
    mock_pm_t *dev = (mock_pm_t *)_dev;

    mock_pm_read_data(dev, &values);
    data->unit = UNIT_GPM3;
    data->scale = -6;
    uint32_t value = values.mc_pm_10;
    phydat_fit(data, (int32_t *)&value, 1);
    return 1;
}

const saul_driver_t mock_pm_saul_driver_mc_pm_10 = {
    .read = read_mc_pm_10,
    .write = saul_notsup,
    .type = SAUL_SENSE_PM,
    .subtype = SAUL_SENSE_PM_10,
};

const saul_driver_t mock_pm_saul_driver_mc_pm_2p5 = {
    .read = read_mc_pm_2p5,
    .write = saul_notsup,
    .type = SAUL_SENSE_PM,
    .subtype = SAUL_SENSE_PM_2p5,
};
