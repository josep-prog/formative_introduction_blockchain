#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#include <time.h>
#include <stddef.h>
#include <openssl/evp.h>

#define MAX_BLOCKS   1000
#define TX_DATA_SIZE 512

typedef struct {
    int index;
    time_t timestamp;

    char book_id[20];
    char book_title[80];

    char member_id[20];
    char member_name[50];

    char librarian_id[20]; /* who recorded the action */

    char action[10];       /* "GENESIS", "BORROWED", "RETURNED" or "OVERDUE" */

    char previous_hash[65]; /* 64 hex chars + '\0' */

    unsigned char signature[72];
    unsigned int signature_length;

    char hash[65];
} Block;


void create_genesis_block(Block *block);

/* Returns 0 if signing failed. */
int create_lending_block(
    Block *block,
    Block *previous_block,
    const char *action,
    const char *book_id,
    const char *book_title,
    const char *member_id,
    const char *member_name,
    const char *librarian_id,
    EVP_PKEY *private_key
);

void calculate_hash(Block *block);

/* On failure, bad_block and reason (may be NULL) say what broke. */
int validate_chain(Block blockchain[], int count, EVP_PKEY *public_key,
                   int *bad_block, const char **reason);

/* load_chain returns the block count, 0 if unreadable, -1 if missing. */
int save_chain(const char *filename, Block blockchain[], int count);
int load_chain(const char *filename, Block blockchain[]);

int find_latest_record(Block blockchain[], int count, const char *book_id);
int find_active_borrow(Block blockchain[], int count, const char *book_id);

void create_transaction_data(Block *block, unsigned char *data, size_t *data_len);

#endif
