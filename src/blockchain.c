#include <stdio.h>
#include <string.h>
#include <time.h>
#include <openssl/sha.h>

#include "blockchain.h"
#include "crypto.h"


void create_transaction_data(Block *block, unsigned char *data, size_t *data_len)
{
    *data_len = snprintf(
        (char *)data,
        500,
        "%d|%ld|%s|%s|%s|%s|%s|%s",
        block->index,
        (long)block->timestamp,
        block->book_id,
        block->book_title,
        block->member_id,
        block->member_name,
        block->action,
        block->previous_hash
    );
}

void create_genesis_block(Block *block)
{
    memset(block, 0, sizeof(Block));

    block->index = 0;
    block->timestamp = time(NULL);
    strcpy(block->action, "GENESIS");

    memset(block->previous_hash, '0', 64);
    block->previous_hash[64] = '\0';

    calculate_hash(block);
}

void calculate_hash(Block *block)
{
    char transaction_data[500];
    unsigned char hash_data[600];
    size_t transaction_len;
    size_t total_len;
    unsigned char digest[SHA256_DIGEST_LENGTH];

    create_transaction_data(block, (unsigned char *)transaction_data, &transaction_len);

    memcpy(hash_data, transaction_data, transaction_len);
    total_len = transaction_len;

    memcpy(hash_data + total_len, &block->signature_length, sizeof(block->signature_length));
    total_len += sizeof(block->signature_length);

    memcpy(hash_data + total_len, block->signature, block->signature_length);
    total_len += block->signature_length;

    SHA256(hash_data, total_len, digest);


    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(&block->hash[i * 2], "%02x", digest[i]);
    }
    block->hash[64] = '\0';
}


static void build_and_sign_block(
    Block *block,
    Block *previous_block,
    const char *book_id,
    const char *book_title,
    const char *member_id,
    const char *member_name,
    const char *action,
    EVP_PKEY *private_key
)
{
    memset(block, 0, sizeof(Block));

    block->index = previous_block->index + 1;
    block->timestamp = time(NULL);

    strcpy(block->book_id, book_id);
    strcpy(block->book_title, book_title);
    strcpy(block->member_id, member_id);
    strcpy(block->member_name, member_name);
    strcpy(block->action, action);
    strcpy(block->previous_hash, previous_block->hash);

    unsigned char data[500];
    size_t data_len;
    size_t signature_len = sizeof(block->signature);

    create_transaction_data(block, data, &data_len);

    if (!sign_data(private_key, data, data_len, block->signature, &signature_len)) {
        printf("ERROR: Could not sign %s transaction.\n", action);
        block->signature_length = 0;
    } else {
        block->signature_length = (unsigned int)signature_len;
    }

    calculate_hash(block);
}

void create_borrow_block(
    Block *block, Block *previous_block,
    const char *book_id, const char *book_title,
    const char *member_id, const char *member_name,
    EVP_PKEY *private_key
)
{
    build_and_sign_block(block, previous_block, book_id, book_title,
                          member_id, member_name, "BORROWED", private_key);
}

void create_return_block(
    Block *block, Block *previous_block,
    const char *book_id, const char *book_title,
    const char *member_id, const char *member_name,
    EVP_PKEY *private_key
)
{
    build_and_sign_block(block, previous_block, book_id, book_title,
                          member_id, member_name, "RETURNED", private_key);
}

int validate_chain(Block blockchain[], int count)
{
    for (int i = 0; i < count; i++) {
        Block copy = blockchain[i];
        calculate_hash(&copy);

        if (strcmp(blockchain[i].hash, copy.hash) != 0) {
            return 0; 
        }

        if (i > 0 && strcmp(blockchain[i].previous_hash, blockchain[i - 1].hash) != 0) {
            return 0;
        }
    }

    return 1;
}

int find_active_borrow(Block blockchain[], int count, const char *book_id)
{
    for (int i = count - 1; i >= 0; i--) {
        if (strcmp(blockchain[i].book_id, book_id) == 0) {
            if (strcmp(blockchain[i].action, "BORROWED") == 0) {
                return i;
            }
            if (strcmp(blockchain[i].action, "RETURNED") == 0) {
                return -1;
            }
        }
    }
    return -1;
}
