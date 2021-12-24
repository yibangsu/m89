#ifndef __M99999999_H__
#define __M99999999_H__

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

#define M89_ENC    0
#define M89_DEC    1

#define M89_ROUND  8

typedef enum {
    M89_OK = 0,
    M89_ERR_INVALID_PARAM,
    M89_ERR_END,
} M89_ERR_T;

typedef struct
{
    uint8_t round;
    uint8_t key[M89_ROUND*4];
} M89_context;

M89_ERR_T M89_init(M89_context *ctx);
M89_ERR_T M89_free(M89_context *ctx);
M89_ERR_T M89_setkey(M89_context *ctx, uint8_t* keyPtr, uint8_t len);
M89_ERR_T M89_enc(M89_context *ctx, uint32_t* in, uint32_t* out);
M89_ERR_T M89_dec(M89_context *ctx, uint32_t* in, uint32_t* out);

#ifdef __cplusplus
}
#endif

#endif // __M99999999_H__
