#include "myls.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

/* Read the block unit from the environment. */
static unsigned long get_block_size(void)
{
    const char *text = getenv("BLOCKSIZE");
    char *end;
    unsigned long value;
    unsigned long factor = 1;

    if (text == NULL || *text == '\0')
        return 512;

    if (*text < '0' || *text > '9')
        return 512;

    errno = 0;
    value = strtoul(text, &end, 10);

    if (errno != 0 || value == 0)
        return 512;

    switch (*end) {
    case 'k':
    case 'K':
        factor = 1024;
        end++;
        break;
    case 'm':
    case 'M':
        factor = 1024UL * 1024;
        end++;
        break;
    case 'g':
    case 'G':
        factor = 1024UL * 1024 * 1024;
        end++;
        break;
    }

    if (*end != '\0' || value > ULONG_MAX / factor)
        return 512;

    value *= factor;
    return value < 512 ? 512 : value;
}

/* Return the index of the first pathname. */
int parse_options(int argc, char **argv, Options *opt)
{
    int ch;

    memset(opt, 0, sizeof(*opt));

    opt->terminal_output = isatty(STDOUT_FILENO);
    opt->replace_nonprint = opt->terminal_output;
    opt->almost_all = (geteuid() == 0);
    opt->block_size = get_block_size();
    opt->time_kind = TIME_MODIFIED;

    opterr = 0;

    while ((ch = getopt(argc, argv,
                        "AacdFfhiklnqRrSstuw")) != -1) {
        switch (ch) {
        case 'A':
            opt->almost_all = 1;
            break;
        case 'a':
            opt->show_all = 1;
            break;
        case 'c':
            opt->time_kind = TIME_CHANGED;
            break;
        case 'd':
            opt->directory_only = 1;
            opt->recursive = 0;
            break;
        case 'F':
            opt->classify = 1;
            break;
        case 'f':
            opt->no_sort = 1;
            break;
        case 'h':
            opt->human_readable = 1;
            opt->block_size = 1024;
            break;
        case 'i':
            opt->show_inode = 1;
            break;
        case 'k':
            opt->human_readable = 0;
            opt->block_size = 1024;
            break;
        case 'l':
            opt->long_format = 1;
            opt->numeric_ids = 0;
            break;
        case 'n':
            opt->long_format = 1;
            opt->numeric_ids = 1;
            break;
        case 'q':
            opt->replace_nonprint = 1;
            break;
        case 'R':
            opt->recursive = 1;
            opt->directory_only = 0;
            break;
        case 'r':
            opt->reverse = 1;
            break;
        case 'S':
            opt->sort_size = 1;
            opt->sort_time = 0;
            break;
        case 's':
            opt->show_blocks = 1;
            break;
        case 't':
            opt->sort_time = 1;
            opt->sort_size = 0;
            break;
        case 'u':
            opt->time_kind = TIME_ACCESSED;
            break;
        case 'w':
            opt->replace_nonprint = 0;
            break;
        default:
            fprintf(stderr,
                "usage: myls [-AacdFfhiklnqRrSstuw] "
                "[file ...]\n");
            return -1;
        }
    }

    return optind;
}
