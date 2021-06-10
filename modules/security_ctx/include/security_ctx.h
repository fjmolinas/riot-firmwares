/*
 * Copyright (C) 2021 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    sys_security_ctx Crypto Context
 * @ingroup     sys
 * @brief       Crypto Context for EDHOC derived keys
 *
 * @{
 *
 * @file
 *
 * @author      Francisco Molina <francois-xavier.molina@inria.fr>
 */

#ifndef SECURITY_CTX_H
#define SECURITY_CTX_H

#include "inttypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#define     SECURITY_CTX_KEY_LEN          (16U)
#define     SECURITY_CTX_COMMON_IV_LEN    (13)
#define     SECURITY_CTX_NONCE_LEN        (13)
#define     SECURITY_CTX_ID_MAX_LEN       (6)

typedef struct security_ctx {
    uint8_t send_ctx_key[SECURITY_CTX_KEY_LEN];
    uint8_t recv_ctx_key[SECURITY_CTX_KEY_LEN];
    uint8_t common_iv[SECURITY_CTX_COMMON_IV_LEN];
    uint8_t* send_id;
    uint8_t* recv_id;
    size_t send_id_len;
    size_t recv_id_len;
    uint16_t seqnr;
} security_ctx_t;

void security_ctx_init(security_ctx_t *ctx, uint8_t *send_id,
                       size_t send_id_len, uint8_t *recv_id,
                       size_t recv_id_len);

int security_ctx_key_gen(security_ctx_t *ctx, uint8_t *salt, size_t salt_len,
                         uint8_t *secret, size_t secret_len);

void security_ctx_gen_nonce(security_ctx_t *ctx, uint8_t *ctx_id,
                            size_t ctx_id_len, uint8_t *nonce);

int security_ctx_encode(security_ctx_t *ctx, uint8_t *data, size_t data_len,
                        uint8_t *buf, size_t buf_len, uint8_t **out);

int security_ctx_decode(security_ctx_t *ctx, uint8_t *in, size_t in_len,
                        uint8_t *buf, size_t buf_len, uint8_t *out, size_t *olen);


#ifdef __cplusplus
}
#endif

#endif /* SECURITY_CTX_H */
/** @} */
