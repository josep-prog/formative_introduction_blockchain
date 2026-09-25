#define _POSIX_C_SOURCE 200809L   /* termios, isatty, getenv */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "registry.h"
#include "blockchain.h"
#include "crypto.h"

#define BOOKS_FILE      "data/books.txt"
#define MEMBERS_FILE    "data/members.txt"
#define LIBRARIANS_FILE "data/librarians.txt"
#define CHAIN_FILE      "data/chain.txt"
#define KEY_FILE        "data/key.pem"
#define PUBLIC_KEY_FILE "data/pub.pem"

#define MAX_LOGIN_ATTEMPTS       3
#define DEFAULT_LOAN_PERIOD_DAYS 14

/* Reads one line without its newline; returns 0 when input is closed. */
static int read_line(char *buf, size_t size)
{
    if (fgets(buf, (int)size, stdin) == NULL) {
        return 0;
    }

    size_t len = strcspn(buf, "\r\n");
    if (buf[len] == '\0' && len == size - 1) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
    }
    buf[len] = '\0';
    return 1;
}

static int prompt_line(const char *prompt, char *buf, size_t size)
{
    printf("%s", prompt);
    fflush(stdout);
    return read_line(buf, size);
}

/* Like prompt_line, but hides what is typed when reading from a terminal. */
static int prompt_secret(const char *prompt, char *buf, size_t size)
{
    struct termios old_attr, new_attr;
    int hide = isatty(STDIN_FILENO) && tcgetattr(STDIN_FILENO, &old_attr) == 0;

    if (hide) {
        new_attr = old_attr;
        new_attr.c_lflag &= ~(tcflag_t)ECHO;
        tcsetattr(STDIN_FILENO, TCSANOW, &new_attr);
    }

    int ok = prompt_line(prompt, buf, size);

    if (hide) {
        tcsetattr(STDIN_FILENO, TCSANOW, &old_attr);
        printf("\n");
    }
    return ok;
}

/* Returns the librarian's index, or -1 after too many failed attempts. */
static int login(Librarian librarians[], int count)
{
    char id[64], pin[64];

    for (int attempt = 1; attempt <= MAX_LOGIN_ATTEMPTS; attempt++) {
        if (!prompt_line("Librarian ID: ", id, sizeof(id)) ||
            !prompt_secret("PIN: ", pin, sizeof(pin))) {
            return -1;
        }

        int index = find_librarian(librarians, count, id);
        int ok = index != -1 && verify_pin(id, pin, librarians[index].pin_hash);
        memset(pin, 0, sizeof(pin));

        if (ok) {
            return index;
        }
        printf("ERROR: Invalid librarian ID or PIN (%d attempt(s) left).\n",
               MAX_LOGIN_ATTEMPTS - attempt);
    }
    return -1;
}

/* Passphrase from LIBRARY_KEY_PASSPHRASE, or asked for. */
static int get_passphrase(char *buf, size_t size)
{
    const char *env = getenv("LIBRARY_KEY_PASSPHRASE");
    if (env != NULL) {
        snprintf(buf, size, "%s", env);
    } else if (!prompt_secret("Key passphrase: ", buf, size)) {
        return 0;
    }

    if (strlen(buf) < MIN_PASSPHRASE_LENGTH) {
        printf("ERROR: The key passphrase must be at least %d characters.\n", MIN_PASSPHRASE_LENGTH);
        return 0;
    }
    return 1;
}

static EVP_PKEY *load_or_create_key(const char *passphrase)
{
    EVP_PKEY *key_pair;
    int status = load_key(KEY_FILE, passphrase, &key_pair);

    if (status == KEY_BAD) {
        printf("ERROR: Could not open %s - wrong passphrase or damaged key file.\n", KEY_FILE);
        return NULL;
    }

    if (status == KEY_LOADED) {
        printf("Digital signing key loaded from %s.\n", KEY_FILE);
        return key_pair;
    }

    if (status == KEY_MISSING) {
        key_pair = generate_key_pair();
        if (key_pair == NULL) {
            printf("ERROR: Could not generate key pair.\n");
            return NULL;
        }
    }

    /* New key, or an old unencrypted one: save it encrypted. */
    if (save_key(key_pair, KEY_FILE, passphrase)) {
        printf("Digital signing key %s and saved encrypted to %s.\n",
               status == KEY_MISSING ? "generated" : "loaded", KEY_FILE);
    } else {
        printf("WARNING: could not save the signing key to %s.\n", KEY_FILE);
    }
    return key_pair;
}

/* Loads data/pub.pem, writing it from the signing key the first time. */
static EVP_PKEY *load_or_create_public_key(EVP_PKEY *key_pair)
{
    EVP_PKEY *public_key = load_public_key(PUBLIC_KEY_FILE);

    if (public_key == NULL) {
        if (!save_public_key(key_pair, PUBLIC_KEY_FILE) ||
            (public_key = load_public_key(PUBLIC_KEY_FILE)) == NULL) {
            printf("ERROR: Could not write the public key to %s.\n", PUBLIC_KEY_FILE);
            return NULL;
        }
        printf("Public verification key saved to %s.\n", PUBLIC_KEY_FILE);
        return public_key;
    }

    if (EVP_PKEY_eq(public_key, key_pair) != 1) {
        printf("ERROR: %s does not belong to the signing key in %s.\n", PUBLIC_KEY_FILE, KEY_FILE);
        EVP_PKEY_free(public_key);
        return NULL;
    }
    return public_key;
}

static long loan_period_seconds(void)
{
    const char *env = getenv("LOAN_PERIOD_SECONDS");   /* override for demos */
    if (env != NULL) {
        char *end;
        long seconds = strtol(env, &end, 10);
        if (*end == '\0' && seconds >= 0) {
            return seconds;
        }
        printf("WARNING: ignoring invalid LOAN_PERIOD_SECONDS '%s'.\n", env);
    }
    return DEFAULT_LOAN_PERIOD_DAYS * 24L * 60 * 60;
}

static void print_block(Block *block, EVP_PKEY *public_key)
{
    printf("\n--------------------------------------------------\n");
    printf("Block #%d  [%s]\n", block->index, block->action);

    if (block->index > 0) {
        printf("  Book    : %s (%s)\n", block->book_title, block->book_id);
        printf("  Member  : %s (%s)\n", block->member_name, block->member_id);
        printf("  By      : %s\n", block->librarian_id);
    }

    printf("  Time    : %s", ctime(&block->timestamp));
    printf("  Hash    : %s\n", block->hash);
    printf("  Prev    : %s\n", block->previous_hash);

    if (block->index == 0) {
        printf("  Signature: n/a (genesis block)\n");
        return;
    }

    unsigned char data[TX_DATA_SIZE];
    size_t data_len;
    create_transaction_data(block, data, &data_len);

    int valid = verify_signature(public_key, data, data_len,
                                  block->signature, block->signature_length);

    printf("  Signature: %s (%u bytes)\n",
           valid ? "VALID" : "INVALID", block->signature_length);
}

/* Returns 1 if the chain is valid. */
static int report_validation(Block blockchain[], int count, EVP_PKEY *public_key)
{
    int bad_block;
    const char *reason;

    if (validate_chain(blockchain, count, public_key, &bad_block, &reason)) {
        printf("Blockchain is VALID - no tampering detected.\n");
        return 1;
    }
    printf("Blockchain is INVALID - tampering detected!\n");
    printf("  Block #%d: %s.\n", bad_block, reason);
    return 0;
}

/* Only a valid chain is written to disk, so the tamper demo can never end up in the file. */
static void save_or_warn(Block blockchain[], int count, EVP_PKEY *public_key)
{
    if (!validate_chain(blockchain, count, public_key, NULL, NULL)) {
        printf("WARNING: chain is invalid, so it was NOT saved to %s.\n", CHAIN_FILE);
        return;
    }
    if (!save_chain(CHAIN_FILE, blockchain, count)) {
        printf("WARNING: could not save the chain to %s.\n", CHAIN_FILE);
    }
}

/* Appends a signed block; returns 1 on success. */
static int append_block(Block blockchain[], int *count, const char *action,
                        Block *details, const char *librarian_id,
                        EVP_PKEY *key_pair, EVP_PKEY *public_key)
{
    if (*count >= MAX_BLOCKS) {
        printf("ERROR: Blockchain is full.\n");
        return 0;
    }

    if (!create_lending_block(&blockchain[*count], &blockchain[*count - 1], action,
                              details->book_id, details->book_title,
                              details->member_id, details->member_name,
                              librarian_id, key_pair)) {
        printf("ERROR: Could not sign %s transaction.\n", action);
        return 0;
    }

    (*count)++;
    save_or_warn(blockchain, *count, public_key);
    return 1;
}

int main(int argc, char *argv[])
{
    /* Helper for creating librarians.txt entries. */
    if (argc == 4 && strcmp(argv[1], "--hash-pin") == 0) {
        char hex[65];
        if (!hash_pin(argv[2], argv[3], hex)) {
            printf("ERROR: Could not hash PIN.\n");
            return 1;
        }
        printf("%s\n", hex);
        return 0;
    }

    /* Checks data/chain.txt with only the public key - no passphrase or login needed. */
    if (argc == 2 && strcmp(argv[1], "--verify") == 0) {
        EVP_PKEY *public_key = load_public_key(PUBLIC_KEY_FILE);
        if (public_key == NULL) {
            printf("ERROR: Could not read the public key from %s.\n", PUBLIC_KEY_FILE);
            return 1;
        }

        static Block blockchain[MAX_BLOCKS];
        int count = load_chain(CHAIN_FILE, blockchain);
        int valid = 0;

        if (count < 0) {
            printf("ERROR: %s does not exist.\n", CHAIN_FILE);
        } else if (count == 0) {
            printf("ERROR: '%s' exists but could not be read.\n", CHAIN_FILE);
        } else {
            printf("Checked %d blocks from %s with %s.\n", count, CHAIN_FILE, PUBLIC_KEY_FILE);
            valid = report_validation(blockchain, count, public_key);
        }

        EVP_PKEY_free(public_key);
        return valid ? 0 : 1;
    }

    Book books[MAX_BOOKS];
    Member members[MAX_MEMBERS];
    Librarian librarians[MAX_LIBRARIANS];

    int book_count = load_books(BOOKS_FILE, books);
    int member_count = load_members(MEMBERS_FILE, members);
    int librarian_count = load_librarians(LIBRARIANS_FILE, librarians);

    if (book_count == 0 || member_count == 0 || librarian_count == 0) {
        return 1;
    }

    printf("Loaded %d books, %d members and %d librarians.\n",
           book_count, member_count, librarian_count);

    char passphrase[128];
    if (!get_passphrase(passphrase, sizeof(passphrase))) {
        return 1;
    }
    EVP_PKEY *key_pair = load_or_create_key(passphrase);
    memset(passphrase, 0, sizeof(passphrase));
    if (key_pair == NULL) {
        return 1;
    }

    EVP_PKEY *public_key = load_or_create_public_key(key_pair);
    if (public_key == NULL) {
        EVP_PKEY_free(key_pair);
        return 1;
    }

    Block blockchain[MAX_BLOCKS];
    int count = load_chain(CHAIN_FILE, blockchain);

    if (count == 0) {
        printf("ERROR: '%s' exists but could not be read. Fix or delete it to continue.\n", CHAIN_FILE);
        return 1;
    } else if (count < 0) {
        create_genesis_block(&blockchain[0]);
        count = 1;
        save_or_warn(blockchain, count, public_key);
        printf("Started a new blockchain.\n");
    } else {
        printf("Loaded %d blocks from %s.\n", count, CHAIN_FILE);
        if (!validate_chain(blockchain, count, public_key, NULL, NULL)) {
            printf("WARNING: the saved blockchain is INVALID - it may have been tampered with!\n");
            report_validation(blockchain, count, public_key);
        }
    }

    int user_index = login(librarians, librarian_count);
    if (user_index == -1) {
        printf("Access denied.\n");
        return 1;
    }
    Librarian *user = &librarians[user_index];
    int is_admin = strcmp(user->role, "ADMIN") == 0;
    printf("Welcome, %s (%s).\n", user->full_name, user->role);

    char input[64];
    int choice = 0;

    do {
        printf("\n LIBRARY MENU \n");
        printf("1. Borrow a book\n");
        printf("2. Return a book\n");
        printf("3. View all lending records\n");
        printf("4. Validate the blockchain\n");
        printf("5. Mark overdue loans\n");
        printf("6. Tamper-detection demo (ADMIN)\n");
        printf("7. Exit\n");

        if (!prompt_line("Choice: ", input, sizeof(input))) {
            printf("\nInput closed. Goodbye.\n");
            break;
        }

        char *end;
        choice = (int)strtol(input, &end, 10);
        if (end == input || *end != '\0') {
            choice = 0;
            printf("Invalid choice.\n");
            continue;
        }

        if (choice == 1) {
            char book_id[64], member_id[64];
            if (!prompt_line("Book ID  : ", book_id, sizeof(book_id)) ||
                !prompt_line("Member ID: ", member_id, sizeof(member_id))) {
                break;
            }

            int book_index = find_book(books, book_count, book_id);
            int member_index = find_member(members, member_count, member_id);

            if (book_index == -1 || member_index == -1) {
                printf("ERROR: Book or Member not found\n");
                continue;
            }

            if (find_active_borrow(blockchain, count, book_id) != -1) {
                printf("ERROR: This book is already on loan.\n");
                continue;
            }

            Block details = {0};
            strcpy(details.book_id, books[book_index].book_id);
            strcpy(details.book_title, books[book_index].title);
            strcpy(details.member_id, members[member_index].member_id);
            strcpy(details.member_name, members[member_index].full_name);

            if (append_block(blockchain, &count, "BORROWED", &details, user->librarian_id,
                             key_pair, public_key)) {
                printf("Borrowed '%s' for %s.\n", details.book_title, details.member_name);
            }

        } else if (choice == 2) {
            char book_id[64], member_id[64];
            if (!prompt_line("Book ID  : ", book_id, sizeof(book_id)) ||
                !prompt_line("Member ID: ", member_id, sizeof(member_id))) {
                break;
            }

            if (find_book(books, book_count, book_id) == -1 ||
                find_member(members, member_count, member_id) == -1) {
                printf("ERROR: Book or Member not found\n");
                continue;
            }

            int loan_index = find_active_borrow(blockchain, count, book_id);
            if (loan_index == -1) {
                printf("ERROR: This book is not currently on loan.\n");
                continue;
            }

            Block details = blockchain[loan_index];
            if (strcmp(details.member_id, member_id) != 0) {
                printf("ERROR: This book is on loan to %s (%s), not %s.\n",
                       details.member_name, details.member_id, member_id);
                continue;
            }

            if (append_block(blockchain, &count, "RETURNED", &details, user->librarian_id,
                             key_pair, public_key)) {
                printf("Returned '%s'.\n", details.book_title);
            }

        } else if (choice == 3) {
            for (int i = 0; i < count; i++) {
                print_block(&blockchain[i], public_key);
            }

        } else if (choice == 4) {
            report_validation(blockchain, count, public_key);

        } else if (choice == 5) {
            long period = loan_period_seconds();
            time_t now = time(NULL);
            int marked = 0;

            for (int b = 0; b < book_count; b++) {
                int latest = find_latest_record(blockchain, count, books[b].book_id);
                if (latest == -1 || strcmp(blockchain[latest].action, "BORROWED") != 0 ||
                    now - blockchain[latest].timestamp < period) {
                    continue;
                }

                Block details = blockchain[latest];
                if (!append_block(blockchain, &count, "OVERDUE", &details, user->librarian_id,
                             key_pair, public_key)) {
                    break;
                }
                printf("OVERDUE: '%s' borrowed by %s (%s).\n",
                       details.book_title, details.member_name, details.member_id);
                marked++;
            }

            printf("%d loan(s) marked overdue (loan period: %ld seconds).\n", marked, period);

        } else if (choice == 6) {
            if (!is_admin) {
                printf("ERROR: Only an ADMIN can run the tamper-detection demo.\n");
                continue;
            }
            if (count < 2) {
                printf("Borrow at least one book first so there's a block to tamper with.\n");
                continue;
            }

            printf("Changing Block #1's book title in memory only, without re-hashing or re-signing.\n");
            strcpy(blockchain[1].book_title, "TAMPERED TITLE");
            report_validation(blockchain, count, public_key);
            printf("(Restart the program to reload the clean chain from disk.)\n");

        } else if (choice == 7) {
            printf("Goodbye.\n");

        } else {
            printf("Invalid choice.\n");
        }

    } while (choice != 7);

    EVP_PKEY_free(public_key);
    EVP_PKEY_free(key_pair);
    return 0;
}
