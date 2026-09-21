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

    /* --- Generate the system's signing key for this session --- */
    EVP_PKEY *key_pair = generate_key_pair();
    if (key_pair == NULL) {
        printf("ERROR: Could not generate key pair.\n");
        return 1;
    }
    printf("Digital signing key generated.\n");

    /* --- Start the chain with the genesis block --- */
    Block blockchain[MAX_BLOCKS];
    int count = 0;
    create_genesis_block(&blockchain[0]);
    count = 1;

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

            printf("Returned '%s'.\n", blockchain[loan_index].book_title);

        } else if (choice == 3) {
            for (int i = 0; i < count; i++) {
                print_block(&blockchain[i], key_pair);
            }

        } else if (choice == 4) {
            if (validate_chain(blockchain, count)) {
                printf("Blockchain is VALID - no tampering detected.\n");
            } else {
                printf("Blockchain is INVALID - tampering detected!\n");
            }

        } else if (choice == 5) {
            if (count < 2) {
                printf("Borrow at least one book first so there's a block to tamper with.\n");
                continue;
            }

            printf("This will edit Block #1's stored book title in memory,\n");
            printf("WITHOUT re-signing or re-hashing it - exactly what an\n");
            printf("attacker trying to rewrite history would do.\n");

            strcpy(blockchain[1].book_title, "TAMPERED TITLE");

            printf("Block #1's book_title has been altered.\n");
            printf("Running chain validation...\n");

            if (validate_chain(blockchain, count)) {
                printf("Blockchain is VALID (unexpected - something's wrong!)\n");
            } else {
                printf("Blockchain is INVALID - tampering detected, as expected.\n");
                printf("(Restart the program to get a clean chain again.)\n");
            }

        } else if (choice == 6) {
            printf("Goodbye.\n");

        } else {
            printf("Invalid choice.\n");
        }

    } while (choice != 6);

    return 0;
}
