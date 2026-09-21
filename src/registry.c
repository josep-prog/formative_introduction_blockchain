#include <stdio.h>
#include <string.h>
#include "registry.h"

/*
 * File format reminder (see books.txt):
 *   BK001,The Money Trap: Lost Illusions Inside the Tech Bubble,Alok Sama
 *   book_id  , title                                          , author
 *
 * We parse this with fscanf() using "scan sets" - %[^,] means "read
 * characters until you hit a comma". This is the standard C idiom for
 * splitting simple CSV-like lines without needing strtok() and its
 * string-mutation quirks.
 *
 * Format string breakdown for a book line: " %19[^,],%79[^,],%49[^\n]"
 *   " "        - skip any leading whitespace/newlines left over from the
 *                previous line (fscanf's %[...] does NOT do this on its
 *                own, so without this leading space, reading would break
 *                after the very first line).
 *   %19[^,]    - read up to 19 chars that are not a comma -> book_id
 *                (19, not 20, to always leave room for the '\0' terminator
 *                that fscanf appends automatically - book_id[20] must hold
 *                19 real characters + 1 null byte).
 *   ,          - literally match and consume the comma separator.
 *   %79[^,]    - up to 79 chars, not a comma -> title
 *   ,          - consume the next comma
 *   %49[^\n]   - up to 49 chars, not a newline -> author (goes to end of
 *                line; author names can't contain commas here so this is
 *                safe for our sample data).
 */

int load_books(const char *filename, Book books[])
{
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        /* Per spec: "print an error message if books.txt is missing." */
        printf("ERROR: Could not open '%s'. Book registry is required to start the system.\n", filename);
        return 0;
    }

    int count = 0;

    while (count < MAX_BOOKS &&
           fscanf(file, " %19[^,],%79[^,],%49[^\n]",
                  books[count].book_id,
                  books[count].title,
                  books[count].author) == 3) {
        count++;
    }

    fclose(file);

    if (count == 0) {
        printf("ERROR: '%s' is empty or badly formatted. No books loaded.\n", filename);
        return 0;
    }

    return count;
}

int load_members(const char *filename, Member members[])
{
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        printf("ERROR: Could not open '%s'. Member registry is required to start the system.\n", filename);
        return 0;
    }

    int count = 0;

    while (count < MAX_MEMBERS &&
           fscanf(file, " %19[^,],%49[^,],%9[^\n]",
                  members[count].member_id,
                  members[count].full_name,
                  members[count].course_code) == 3) {
        count++;
    }

    fclose(file);

    if (count == 0) {
        printf("ERROR: '%s' is empty or badly formatted. No members loaded.\n", filename);
        return 0;
    }

    return count;
}

int find_book(Book books[], int count, const char *book_id)
{
    for (int i = 0; i < count; i++) {
        /*
         * strcmp, not strncmp: IDs must match EXACTLY. Using a "starts
         * with" comparison here would let "BK001EXTRA" match "BK001",
         * which is exactly the kind of sloppy validation bug that defeats
         * the point of having a registry at all.
         */
        if (strcmp(books[i].book_id, book_id) == 0) {
            return i;
        }
    }

    return -1; /* Not found - caller must treat this as a hard rejection. */
}

int find_member(Member members[], int count, const char *member_id)
{
    for (int i = 0; i < count; i++) {
        if (strcmp(members[i].member_id, member_id) == 0) {
            return i;
        }
    }

    return -1;
}
