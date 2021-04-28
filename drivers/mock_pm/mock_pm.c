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
 * @brief       Implementation of mock_pm dust sensor
 *
 * @author      Francisco Molina <francois-xavier.molina@inria.fr>
 * @}
 */

#include <assert.h>
#include <string.h>

#include "mock_pm.h"
#include "random.h"
#include "ztimer.h"

static void _sample_timer_cb(void *arg)
{
    mock_pm_t *dev = (mock_pm_t *)arg;
    /* schedule next sample */
    ztimer_set(ZTIMER_USEC, &dev->_sampler, CONFIG_MOCK_PM_SAMPLE_TIME);

    uint16_t pm2p5 = random_uint32_range(0, CONFIG_MOCK_PM_2P5_MAX);
    uint16_t pm10 = random_uint32_range(0, CONFIG_MOCK_PM_10_MAX);

    dev->data.mc_pm_10 =
        (uint16_t)((pm10 + (uint32_t)(CONFIG_MOCK_PM_EXP_WEIGHT - 1) *
                    dev->data.mc_pm_10) / CONFIG_MOCK_PM_EXP_WEIGHT);
    dev->data.mc_pm_2p5 =
        (uint16_t)((pm2p5 + (uint32_t)(CONFIG_MOCK_PM_EXP_WEIGHT - 1) *
                    dev->data.mc_pm_2p5) / CONFIG_MOCK_PM_EXP_WEIGHT);
}

void mock_pm_read_data(mock_pm_t *dev, mock_pm_data_t *data)
{
    assert(dev);
    unsigned int state = irq_disable();
    data->mc_pm_10 = dev->data.mc_pm_10;
    data->mc_pm_2p5 = dev->data.mc_pm_2p5;
    irq_restore(state);
}

void mock_pm_init(mock_pm_t *dev)
{
    assert(dev);
    /* setup timer */
    dev->_sampler.callback = _sample_timer_cb;
    dev->_sampler.arg = dev;
    ztimer_set(ZTIMER_USEC, &dev->_sampler, CONFIG_MOCK_PM_SAMPLE_TIME);
}
