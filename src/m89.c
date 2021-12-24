#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>    /* for exit */
#include <getopt.h>    /* for getopt_long; POSIX standard getopt is in unistd.h */


#include "m89.h"
#include "Config.h"

#define M89_DEBUG

#ifdef M89_DEBUG
#define M89_log(...)   fprintf(stdout, __VA_ARGS__)
#else
#define M89_log(...)
#endif

#define M_BASE  (0x5F5E100)

M89_ERR_T M89_init(M89_context *ctx)
{
    if (!ctx)
    {
        M89_log("%s ctx is NULL\n", __func__);
        return M89_ERR_INVALID_PARAM;
    }

    ctx->round = M89_ROUND;

    return M89_OK;
}

M89_ERR_T M89_free(M89_context *ctx)
{
    if (!ctx)
    {
        M89_log("%s ctx is NULL\n", __func__);
        return M89_ERR_INVALID_PARAM;
    }

    memset(ctx, 0, sizeof(M89_context));

    return M89_OK;
}

M89_ERR_T M89_setkey(M89_context *ctx, uint8_t* keyPtr, uint8_t len)
{
    if (!ctx || !keyPtr)
    {
        M89_log("%s ctx or keyPtr is NULL\n", __func__);
        return M89_ERR_INVALID_PARAM;
    }

    /* set key and advoid all zero key */
    uint8_t* ctxKeyPtr = (uint8_t*)ctx->key;
    uint8_t ctxKeyLen = sizeof(((M89_context *)0)->key)  - 1;
    len = len < ctxKeyLen? len: ctxKeyLen;
    memset(ctxKeyPtr, (ctxKeyLen - len) & 0xFF, ctxKeyLen);
    memcpy(ctxKeyPtr, keyPtr, len);
    ctxKeyPtr[ctxKeyLen] = len;

    return M89_OK;
}

static uint32_t get_key32(uint8_t* k)
{
    uint32_t ret = 0;
    ret = (k[0] << 24) | (k[1] << 16) | (k[2] << 8) | k[3];
    return ret;
}

static uint32_t M89_enc_run(uint32_t a, uint32_t k)
{
    uint64_t b;
    
    /* plus */
    k = k % M_BASE;
    a = (a + k) % M_BASE;

    /* shift left */
    uint8_t c = (a / 10000000) % 10;
    a = (a % 10000000) * 10 + c;

    /* s-box */
    b = a;
    b *= 0x3F940AB;
    
    a = b % M_BASE;

    return a;
}

M89_ERR_T M89_enc(M89_context *ctx, uint32_t* in, uint32_t* out)
{
    if (!ctx || !in || !out)
    {
        M89_log("%s ctx or in or out is NULL\n", __func__);
        return M89_ERR_INVALID_PARAM;
    }

    /* run round */
    for (int i = 0; i < M89_ROUND; i++)
    {
        uint32_t k = get_key32(ctx->key + 4*i);
        *in = M89_enc_run(*in, k);
    }

    /* merge out */
    *out = *in;

    return M89_OK;
}

static uint32_t M89_dec_run(uint32_t a, uint32_t k)
{
    /* s-box */
    uint64_t b = a;
    b *= 0x3;
    a = b % M_BASE;

    /* shift right */
    uint8_t c = a % 10;
    a = c * 10000000 + (a / 10);

    /* minus */
    k = k % M_BASE;
    if (a >= k)
    {
        a = a - k;
    }
    else
    {
        a = a + (M_BASE - k);
    }

    return a;
}

M89_ERR_T M89_dec(M89_context *ctx, uint32_t* in, uint32_t* out)
{
    if (!ctx || !in || !out)
    {
        M89_log("%s ctx or in or out is NULL\n", __func__);
        return M89_ERR_INVALID_PARAM;
    }

    /* run round */
    for (int i = M89_ROUND - 1; i >= 0 ; i--)
    {
        uint32_t k = get_key32(ctx->key + 4*i);
        *in = M89_dec_run(*in, k);
    }

    /* merge out */
    *out = *in;

    return M89_OK;
}

#define ERR_OUT(ret) \
if (ret != M89_OK) \
{ M89_log("Error out at line %d\n", __LINE__); }

static uint32_t str2uint32(char *string)
{
    uint32_t ret = 0;
    int len = strlen(string);
    if (len > 2 && string[0] == '0' && (string[1] == 'x' || string[1] == 'X'))
    {
        for (int i = 2; i < len; i++)
        {
            if (string[i] >= '0' && string[i] <= '9')
            {
                ret = ret * 16 + (string[i] - '0');
            }
            else if (string[i] >= 'a' && string[i] <= 'f')
            {
                ret = ret * 16 + 10 + (string[i] - 'a');
            }
            else if (string[i] >= 'A' && string[i] <= 'F')
            {
                ret = ret * 16 + 10 + (string[i] - 'A');
            }
            else
            {
                ret = ret * 16;
            }
        }
    }
    else if (len > 2 && string[0] == '0' && (string[1] == 'h' || string[1] == 'H'))
    {
        for (int i = 2; i < len; i++)
        {
            if (string[i] >= '0' && string[i] <= '7')
            {
                ret = ret * 8 + (string[i] - '0');
            }
            else
            {
                ret = ret * 8;
            }
        }
    }
    else if (len > 2 && string[0] == '0' && (string[1] == 'b' || string[1] == 'B'))
    {
        for (int i = 2; i < len; i++)
        {
            if (string[i] >= '0' && string[i] <= '1')
            {
                ret = (ret << 1) + (string[i] - '0');
            }
            else
            {
                ret = ret << 1;
            }
        }
    }
    else if (len > 2 && string[0] == '0' && (string[1] == 'd' || string[1] == 'D'))
    {
        for (int i = 2; i < len; i++)
        {
            if (string[i] >= '0' && string[i] <= '9')
            {
                ret = ret * 10 + (string[i] - '0');
            }
            else
            {
                ret = ret * 10;
            }
        }
    }
    else
    {
        for (int i = 0; i < len; i++)
        {
            if (string[i] >= '0' && string[i] <= '9')
            {
                ret = ret * 10 + (string[i] - '0');
            }
            else
            {
                ret = ret * 10;
            }
        }
    }
    
    return ret;
}

#define DEFAULT_IN  "0x12345678"
#define DEFAULT_KEY "abcdefghijklmnopqrstuvwxyz"

static int check_enc_dec(uint8_t bOutText, char* key)
{
    int err_count = 0;
    FILE *f_ptr = NULL;
    M89_ERR_T ret = M89_ERR_END;
    M89_context ctx;

    ret = M89_init(&ctx);
    ERR_OUT(ret);
    ret = M89_setkey(&ctx, key,strlen(key));


    // test weak key
    // uint8_t key[32];
    // memset(key, 0xFF, 32);
    // ret = M89_setkey(&ctx, key, 32);

    ERR_OUT(ret);

    for (uint32_t i = 0; i < 100; i++)
    {
        char file_name[64];
        char buffer[1024];

        if (bOutText)
        {
            sprintf(file_name, "enc_file_%04d.txt", i);
            f_ptr = fopen(file_name,"w+");
            if (!f_ptr)
            {
                M89_log("can't open file %s\n", file_name);
            }
        }
        
        for (uint32_t j = i*1000000; j < (i+1)*1000000; j++)
        {
            uint32_t in = j;
            uint32_t out;
            uint32_t enc;
            ret = M89_enc(&ctx, &in, &out);
            ERR_OUT(ret);
            enc = out;
            ret = M89_dec(&ctx, &out, &in);
            ERR_OUT(ret);
            if (j != in)
            {
                M89_log("crypt error at %d\n", j);
                err_count++;
            }

            if (bOutText && f_ptr)
            {
                memset(buffer, 0, sizeof(buffer));
                if (j != in)
                {
                    sprintf(buffer, "error at %08d -> %08d\r\n", j, enc);
                }
                else
                {
                    sprintf(buffer, "%08d -> %08d\r\n", j, enc);
                }
                fwrite(buffer, strlen(buffer), 1, f_ptr);
            }
        }

        if (bOutText && f_ptr)
        {
            fclose(f_ptr);
            f_ptr = NULL;
        }
    }
    
    return err_count;
}

extern char *optarg;
extern int optind, opterr, optopt;

void main(int argc, char** argv)
{
    char* in = DEFAULT_IN;
    char* key = DEFAULT_KEY;
    uint8_t bTest = 0;
    uint8_t bHelp = 0;
    uint8_t bOutText = 0;
    int c;

    static struct option long_options[] = {
    /*   NAME       ARGUMENT           FLAG  SHORTNAME */
        {"input",   required_argument, NULL, 'i'},
        {"key",     required_argument, NULL, 'k'},
        {"test",    no_argument,       NULL, 't'},
        {"help",    no_argument,       NULL, 'h'},
        {"out-text",no_argument,       NULL, 'o'},
        {NULL,      0,                 NULL, 0},
    };

    int option_index = 0;
    while ((c = getopt_long(argc, argv, "i:k:oth?",
            long_options, &option_index)) != -1) 
    {
        switch (c)
        {
        case 'i':
            in = optarg;
            break;
        case 'k':
            key = optarg;
            break;
        case 't':
            bTest = 1;
            break;
        case 'o':
            bOutText = 1;
            break;
        /* pass through */
        case 'h':
        case '?':
        default:
            bHelp = 1;
            break;
        }
    };

    if (bHelp)
    {
        M89_log("Usage:\n" \
        " %s [option]\n" \
        "  --help: print this uasge\n" \
        "  --test: test encryption and decryption\n" \
        "  --input <input value>: encrypt/decrypt the input value\n" \
        "  --key <key value>: set crypto key\n", argv[0]);
        return;
    }

    if (bTest)
    {
        int err_count = check_enc_dec(bOutText, key);
        M89_log("err_count = %d\n", err_count);
        return;
    }

    M89_log("in=%s, key=%s, bTest=%d\n", in, key, bTest);

    uint32_t input = str2uint32(in);
    uint32_t output;
    M89_ERR_T ret = M89_ERR_END;
    M89_context ctx;
    ret = M89_init(&ctx);
    ERR_OUT(ret);
    ret = M89_setkey(&ctx, key, strlen(key));
    ERR_OUT(ret);
    M89_log("enc input is %08x(%d)\n", input, input);
    ret = M89_enc(&ctx, &input, &output);
    ERR_OUT(ret);
    M89_log("enc out is %08x(%d)\n", output, output);
    ret = M89_dec(&ctx, &output, &input);
    ERR_OUT(ret);
    M89_log("dec out is %08x(%d)\n", input, input);
    ret = M89_free(&ctx);
    ERR_OUT(ret);
}
