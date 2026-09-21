#ifndef REGISTRY_H
#define REGISTRY_H


#define MAX_BOOKS   100
#define MAX_MEMBERS 100

typedef struct {
    char book_id[20];
    char title[80];
    char author[50];
} Book;


// A Member as it appears in the library's membership roll (members.txt).

typedef struct {
    char member_id[20];
    char full_name[50];
    char course_code[10];
} Member;


int load_books(const char *filename, Book books[]);
int load_members(const char *filename, Member members[]);

int find_book(Book books[], int count, const char *book_id);
int find_member(Member members[], int count, const char *member_id);

#endif