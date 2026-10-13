#ifndef MYLS_H
#define MYLS_H

#include <sys/types.h>
#include <sys/stat.h>
#include <stddef.h>
#include <time.h>

typedef enum {
    TIME_MODIFIED,
    TIME_ACCESSED,
    TIME_CHANGED
} TimeKind;

typedef struct {
    int show_all;
    int almost_all;
    int directory_only;
    int recursive;
    int classify;
    int no_sort;
    int reverse;
    int sort_size;
    int sort_time;
    int long_format;
    int numeric_ids;
    int show_inode;
    int show_blocks;
    int human_readable;
    int replace_nonprint;
    int terminal_output;
    unsigned long block_size;
    TimeKind time_kind;
} Options;

typedef struct {
    char *name;
    char *path;
    struct stat st;
} Entry;

extern int exit_status;

int parse_options(int argc, char **argv, Options *opt);

void list_operands(int count, char **paths,
                   const Options *opt);

void sort_entries(Entry *entries, size_t count,
                  const Options *opt);

void print_entry(const Entry *entry, const Options *opt);

void print_total(const Entry *entries, size_t count,
                 const Options *opt);

time_t entry_time(const Entry *entry, const Options *opt);

#endif
