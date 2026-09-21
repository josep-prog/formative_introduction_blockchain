#include <stdio.h>
#include <openssl/evp.h>
#include <openssl/ec.h>

#include "crypto.h"

EVP_PKEY *generate_key_pair(void)
{
    EVP_PKEY_CTX *ctx;
    EVP_PKEY *key_pair = NULL;

    ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, NULL);
    if (ctx == NULL) {
        return NULL;
    }

    if (EVP_PKEY_keygen_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return NULL;
    }

    if (EVP_PKEY_CTX_set_ec_paramgen_curve_nid(ctx, NID_X9_62_prime256v1) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return NULL;
    }

    if (EVP_PKEY_keygen(ctx, &key_pair) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return NULL;
    }

    EVP_PKEY_CTX_free(ctx);

    return key_pair;
}

int sign_data(
    EVP_PKEY *private_key,
    const unsigned char *data,
    size_t data_len,
    unsigned char *signature,
    size_t *signature_len
)
{
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (ctx == NULL) {
        return 0;
    }


    if (EVP_DigestSignInit(ctx, NULL, EVP_sha256(), NULL, private_key) <= 0) {
        EVP_MD_CTX_free(ctx);
        return 0;
    }

    if (EVP_DigestSignUpdate(ctx, data, data_len) <= 0) {
        EVP_MD_CTX_free(ctx);
        return 0;
    }

    if (EVP_DigestSignFinal(ctx, NULL, signature_len) <= 0) {
        EVP_MD_CTX_free(ctx);
        return 0;
    }

    if (*signature_len > 72) {
        EVP_MD_CTX_free(ctx);
        return 0;
    }

    if (EVP_DigestSignFinal(ctx, signature, signature_len) <= 0) {
        EVP_MD_CTX_free(ctx);
        return 0;
    }

    EVP_MD_CTX_free(ctx);
    return 1;
}

int verify_signature(
    EVP_PKEY *public_key,
    const unsigned char *data,
    size_t data_len,
    const unsigned char *signature,
    size_t signature_len
)
{
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (ctx == NULL) {
        return 0;
    }

    if (EVP_DigestVerifyInit(ctx, NULL, EVP_sha256(), NULL, public_key) <= 0) {
        EVP_MD_CTX_free(ctx);
        return 0;
    }

    if (EVP_DigestVerifyUpdate(ctx, data, data_len) <= 0) {
        EVP_MD_CTX_free(ctx);
        return 0;
    }

    int result = EVP_DigestVerifyFinal(ctx, signature, signature_len);

    EVP_MD_CTX_free(ctx);

    return (result == 1) ? 1 : 0;
}
