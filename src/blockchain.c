#include <stdio.h>
#include <stdlib.h>
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

int validate_chain(Block blockchain[], int count, EVP_PKEY *public_key)
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

        /* The genesis block is not signed; every other block must be. */
        if (i > 0) {
            unsigned char data[500];
            size_t data_len;
            create_transaction_data(&blockchain[i], data, &data_len);

            if (!verify_signature(public_key, data, data_len,
                                  blockchain[i].signature, blockchain[i].signature_length)) {
                return 0;
            }
        }
    }

    return 1;
}

/*
 * Chain file format: one block per line, fields separated by '|':
 *   index|timestamp|book_id|book_title|member_id|member_name|action|previous_hash|signature_hex|hash
 * Plain text on purpose, so you can open it and edit a block to try the tamper detection.
 */
int save_chain(const char *filename, Block blockchain[], int count)
{
    char temp_name[256];
    snprintf(temp_name, sizeof(temp_name), "%s.tmp", filename);

    FILE *file = fopen(temp_name, "w");
    if (file == NULL) {
        return 0;
    }

    for (int i = 0; i < count; i++) {
        Block *b = &blockchain[i];

        fprintf(file, "%d|%ld|%s|%s|%s|%s|%s|%s|",
                b->index, (long)b->timestamp, b->book_id, b->book_title,
                b->member_id, b->member_name, b->action, b->previous_hash);

        for (unsigned int j = 0; j < b->signature_length; j++) {
            fprintf(file, "%02x", b->signature[j]);
        }

        fprintf(file, "|%s\n", b->hash);
    }

    if (fclose(file) != 0) {
        return 0;
    }

    /* Write to a temp file first so a crash never leaves a half-written chain. */
    return rename(temp_name, filename) == 0;
}

/* Cuts the next '|'-separated field out of *cursor (empty fields are allowed). */
static char *next_field(char **cursor)
{
    char *start = *cursor;
    if (start == NULL) {
        return NULL;
    }

    char *bar = strchr(start, '|');
    if (bar != NULL) {
        *bar = '\0';
        *cursor = bar + 1;
    } else {
        *cursor = NULL;
    }
    return start;
}

static int hex_value(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

/* Parses one line into a block. Returns 1 if the line is well formed. */
static int parse_block_line(char *line, Block *block)
{
    char *cursor = line;
    char *index_text     = next_field(&cursor);
    char *time_text      = next_field(&cursor);
    char *book_id        = next_field(&cursor);
    char *book_title     = next_field(&cursor);
    char *member_id      = next_field(&cursor);
    char *member_name    = next_field(&cursor);
    char *action         = next_field(&cursor);
    char *previous_hash  = next_field(&cursor);
    char *signature_text = next_field(&cursor);
    char *hash           = next_field(&cursor);

    /* Too few fields, or too many (a leftover '|' in the last field). */
    if (hash == NULL || cursor != NULL) {
        return 0;
    }

    if (strlen(book_id) >= sizeof(block->book_id) ||
        strlen(book_title) >= sizeof(block->book_title) ||
        strlen(member_id) >= sizeof(block->member_id) ||
        strlen(member_name) >= sizeof(block->member_name) ||
        strlen(action) >= sizeof(block->action) ||
        strlen(previous_hash) != 64 || strlen(hash) != 64) {
        return 0;
    }

    size_t hex_len = strlen(signature_text);
    if (hex_len % 2 != 0 || hex_len / 2 > sizeof(block->signature)) {
        return 0;
    }

    memset(block, 0, sizeof(Block));
    block->index = atoi(index_text);
    block->timestamp = (time_t)atol(time_text);
    strcpy(block->book_id, book_id);
    strcpy(block->book_title, book_title);
    strcpy(block->member_id, member_id);
    strcpy(block->member_name, member_name);
    strcpy(block->action, action);
    strcpy(block->previous_hash, previous_hash);
    strcpy(block->hash, hash);

    for (size_t i = 0; i < hex_len / 2; i++) {
        int high = hex_value(signature_text[i * 2]);
        int low  = hex_value(signature_text[i * 2 + 1]);
        if (high < 0 || low < 0) {
            return 0;
        }
        block->signature[i] = (unsigned char)(high * 16 + low);
    }
    block->signature_length = (unsigned int)(hex_len / 2);

    return 1;
}

int load_chain(const char *filename, Block blockchain[])
{
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        return -1;
    }

    char line[1024];
    int count = 0;

    while (fgets(line, sizeof(line), file) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0') {
            continue;
        }

        if (count >= MAX_BLOCKS || !parse_block_line(line, &blockchain[count])) {
            fclose(file);
            return 0;
        }
        count++;
    }

    fclose(file);
    return count;
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
