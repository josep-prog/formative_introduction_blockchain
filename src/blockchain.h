#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#include <time.h>
#include <stddef.h>
#include <openssl/evp.h>

#define MAX_BLOCKS 1000

typedef struct {
    int index;
    time_t timestamp;

    char book_id[20];
    char book_title[80];

    char member_id[20];
    char member_name[50];

    char action[10];       /* "GENESIS", "BORROWED", or "RETURNED" */

    char previous_hash[65]; /* 64 hex chars + '\0' */

    unsigned char signature[72];
    unsigned int signature_length;

    char hash[65];
} Block;


void create_genesis_block(Block *block);

void create_borrow_block(
    Block *block,
    Block *previous_block,
    const char *book_id,
    const char *book_title,
    const char *member_id,
    const char *member_name,
    EVP_PKEY *private_key
);

void create_return_block(
    Block *block,
    Block *previous_block,
    const char *book_id,
    const char *book_title,
    const char *member_id,
    const char *member_name,
    EVP_PKEY *private_key
);

void calculate_hash(Block *block);

int validate_chain(Block blockchain[], int count, EVP_PKEY *public_key);

/* save_chain: 1 on success, 0 on failure.
 * load_chain: number of blocks loaded, 0 if the file is unusable, -1 if it does not exist. */
int save_chain(const char *filename, Block blockchain[], int count);
int load_chain(const char *filename, Block blockchain[]);

int find_active_borrow(Block blockchain[], int count, const char *book_id);

void create_transaction_data(Block *block, unsigned char *data, size_t *data_len);

#endif
