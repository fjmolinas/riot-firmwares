#ifndef SAUL_COAP_H
#define SAUL_COAP_H

#include <inttypes.h>

#include "saul.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Supported saul sensors list
 */
const uint8_t saul_sense_supported[][2] = {
    {SAUL_SENSE_CO2, SAUL_CLASS_ANY},
    {SAUL_SENSE_HUM, SAUL_CLASS_ANY},
    {SAUL_SENSE_LIGHT, SAUL_CLASS_ANY},
    {SAUL_SENSE_PM, SAUL_SENSE_PM_10},
    {SAUL_SENSE_PM, SAUL_SENSE_PM_1},
    {SAUL_SENSE_PM, SAUL_SENSE_PM_2p5},
    {SAUL_SENSE_PRESS, SAUL_CLASS_ANY},
    {SAUL_SENSE_TEMP, SAUL_CLASS_ANY},
    {SAUL_SENSE_TVOC, SAUL_CLASS_ANY},
};

/**
 * @brief   Send a SenML JSON encode saul sensor measurement
 *
 * @param[in] args[0]   saulreg type to match
 * @param[in] args[1]   saulreg subtype to match, use SAUL_CLASS_ANY
 *                      if don't care
 */
void saul_sense_coap_send(void *args);

/**
 * @brief   Start saul sense subtree listener
 *
 */
void saul_sense_coap_init(void);

#ifdef __cplusplus
}
#endif

#endif /* SAUL_COAP_H */
