#include <stdio.h>
#include <string.h>

#include "registry.h"
#include "blockchain.h"
#include "crypto.h"

static void print_block(Block *block, EVP_PKEY *key_pair)
{
    printf("\n--------------------------------------------------\n");
    printf("Block #%d  [%s]\n", block->index, block->action);

    if (block->index > 0) {
        printf("  Book    : %s (%s)\n", block->book_title, block->book_id);
        printf("  Member  : %s (%s)\n", block->member_name, block->member_id);
    }

    printf("  Time    : %s", ctime(&block->timestamp));
    printf("  Hash    : %s\n", block->hash);
    printf("  Prev    : %s\n", block->previous_hash);

    if (block->index == 0) {
        printf("  Signature: n/a (genesis block)\n");
        return;
    }

    unsigned char data[500];
    size_t data_len;
    create_transaction_data(block, data, &data_len);

    int valid = verify_signature(key_pair, data, data_len,
                                  block->signature, block->signature_length);

    printf("  Signature: %s (%u bytes)\n",
           valid ? "VALID" : "INVALID", block->signature_length);
}

#define CHAIN_FILE "data/chain.txt"
#define KEY_FILE   "data/key.pem"

/* Only a valid chain is written to disk, so the tamper demo can never end up in the file. */
static void save_or_warn(Block blockchain[], int count, EVP_PKEY *key_pair)
{
    if (!validate_chain(blockchain, count, key_pair)) {
        printf("WARNING: chain is invalid, so it was NOT saved to %s.\n", CHAIN_FILE);
        return;
    }
    if (!save_chain(CHAIN_FILE, blockchain, count)) {
        printf("WARNING: could not save the chain to %s.\n", CHAIN_FILE);
    }
}

int main(void)
{
    Book books[MAX_BOOKS];
    Member members[MAX_MEMBERS];

    /* --- Load registries (requirement: registries loaded from file) --- */
    int book_count = load_books("data/books.txt", books);
    int member_count = load_members("data/members.txt", members);

    if (book_count == 0 || member_count == 0) {
        /* load_books/load_members already printed the specific error. */
        return 1;
    }

    printf("Loaded %d books and %d members.\n", book_count, member_count);

    /* --- Load the signing key, or generate and save one on the first run --- */
    EVP_PKEY *key_pair = load_key(KEY_FILE);
    if (key_pair != NULL) {
        printf("Digital signing key loaded from %s.\n", KEY_FILE);
    } else {
        key_pair = generate_key_pair();
        if (key_pair == NULL) {
            printf("ERROR: Could not generate key pair.\n");
            return 1;
        }
        if (save_key(key_pair, KEY_FILE)) {
            printf("Digital signing key generated and saved to %s.\n", KEY_FILE);
        } else {
            printf("Digital signing key generated (WARNING: could not save it to %s).\n", KEY_FILE);
        }
    }

    /* --- Load the saved chain, or start a new one with the genesis block --- */
    Block blockchain[MAX_BLOCKS];
    int count = load_chain(CHAIN_FILE, blockchain);

    if (count == 0) {
        /* Don't start a fresh chain over a file we couldn't read. */
        printf("ERROR: '%s' exists but could not be read. Fix or delete it to continue.\n", CHAIN_FILE);
        return 1;
    } else if (count < 0) {
        create_genesis_block(&blockchain[0]);
        count = 1;
        save_or_warn(blockchain, count, key_pair);
        printf("Started a new blockchain.\n");
    } else {
        printf("Loaded %d blocks from %s.\n", count, CHAIN_FILE);
        if (!validate_chain(blockchain, count, key_pair)) {
            printf("WARNING: the saved blockchain is INVALID - it may have been tampered with!\n");
        }
    }

    int choice;

    do {
        printf("\n LIBRARY MENU \n");
        printf("1. Borrow a book\n");
        printf("2. Return a book\n");
        printf("3. View all lending records\n");
        printf("4. Validate the blockchain\n");
        printf("5. Tamper-detection demo\n");
        printf("6. Exit\n");
        printf("Choice: ");

        int scan_result = scanf("%d", &choice);
        if (scan_result == EOF) {
            /* stdin closed: nothing more can be read, so exit cleanly. */
            printf("\nInput closed. Goodbye.\n");
            break;
        }
        if (scan_result != 1) {
            /* Non-numeric input: clear it out and re-prompt. */
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }

        if (choice == 1) {
            // borrow
            char book_id[20], member_id[20];
            printf("Book ID  : ");
            if (scanf("%19s", book_id) != 1) break;
            printf("Member ID: ");
            if (scanf("%19s", member_id) != 1) break;

            int book_index = find_book(books, book_count, book_id);
            int member_index = find_member(members, member_count, member_id);

            if (book_index == -1 || member_index == -1) {
                /* Exact wording required by the assignment spec. */
                printf("ERROR: Book or Member not found\n");
                continue;
            }

            if (find_active_borrow(blockchain, count, book_id) != -1) {
                printf("ERROR: This book is already on loan.\n");
                continue;
            }

            if (count >= MAX_BLOCKS) {
                printf("ERROR: Blockchain is full.\n");
                continue;
            }

            create_borrow_block(
                &blockchain[count], &blockchain[count - 1],
                books[book_index].book_id, books[book_index].title,
                members[member_index].member_id, members[member_index].full_name,
                key_pair
            );
            count++;
            save_or_warn(blockchain, count, key_pair);

            printf("Borrowed '%s' for %s.\n",
                   books[book_index].title, members[member_index].full_name);

        } else if (choice == 2) {
            char book_id[20];
            printf("Book ID: ");
            if (scanf("%19s", book_id) != 1) break;

            int book_index = find_book(books, book_count, book_id);
            if (book_index == -1) {
                printf("ERROR: Book or Member not found\n");
                continue;
            }

            int loan_index = find_active_borrow(blockchain, count, book_id);
            if (loan_index == -1) {
                printf("ERROR: This book is not currently on loan.\n");
                continue;
            }

            if (count >= MAX_BLOCKS) {
                printf("ERROR: Blockchain is full.\n");
                continue;
            }

            create_return_block(
                &blockchain[count], &blockchain[count - 1],
                blockchain[loan_index].book_id, blockchain[loan_index].book_title,
                blockchain[loan_index].member_id, blockchain[loan_index].member_name,
                key_pair
            );
            count++;
            save_or_warn(blockchain, count, key_pair);

            printf("Returned '%s'.\n", blockchain[loan_index].book_title);

        } else if (choice == 3) {
            for (int i = 0; i < count; i++) {
                print_block(&blockchain[i], key_pair);
            }

        } else if (choice == 4) {
            if (validate_chain(blockchain, count, key_pair)) {
                printf("Blockchain is VALID - no tampering detected.\n");
            } else {
                printf("Blockchain is INVALID - tampering detected!\n");
            }

        } else if (choice == 5) {
            if (count < 2) {
                printf("Borrow at least one book first so there's a block to tamper with.\n");
                continue;
            }

            printf("This will edit Block #1's stored book title in memory\n");
            printf("(the saved file is left untouched), WITHOUT re-signing or\n");
            printf("re-hashing it - exactly what an attacker trying to rewrite\n");
            printf("history would do.\n");

            strcpy(blockchain[1].book_title, "TAMPERED TITLE");

            printf("Block #1's book_title has been altered.\n");
            printf("Running chain validation...\n");

            if (validate_chain(blockchain, count, key_pair)) {
                printf("Blockchain is VALID (unexpected - something's wrong!)\n");
            } else {
                printf("Blockchain is INVALID - tampering detected, as expected.\n");
                printf("(Restart the program to reload the clean chain from disk.)\n");
            }

        } else if (choice == 6) {
            printf("Goodbye.\n");

        } else {
            printf("Invalid choice.\n");
        }

    } while (choice != 6);

    return 0;
}
