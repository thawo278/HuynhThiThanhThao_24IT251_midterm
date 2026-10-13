#include "myls.h"

#include <stdlib.h>
#include <string.h>

/* qsort has no context parameter; this program is single-threaded. */
static const Options *sort_options;

/* Select the timestamp used by both sorting and printing. */
time_t entry_time(const Entry *entry, const Options *opt)
{
    switch (opt->time_kind) {
    case TIME_ACCESSED:
        return entry->st.st_atime;
    case TIME_CHANGED:
        return entry->st.st_ctime;
    default:
        return entry->st.st_mtime;
    }
}

/* Compare nanoseconds when two timestamps share the same second. */
static long entry_nsec(const Entry *entry, const Options *opt)
{
    switch (opt->time_kind) {
    case TIME_ACCESSED:
        return entry->st.st_atim.tv_nsec;
    case TIME_CHANGED:
        return entry->st.st_ctim.tv_nsec;
    default:
        return entry->st.st_mtim.tv_nsec;
    }
}

static int compare_entries(const void *left, const void *right)
{
    const Entry *a = left;
    const Entry *b = right;
    const Options *opt = sort_options;
    int result = 0;

    if (opt->sort_size) {
        if (a->st.st_size > b->st.st_size)
            result = -1;
        else if (a->st.st_size < b->st.st_size)
            result = 1;
    } else if (opt->sort_time) {
        time_t ta = entry_time(a, opt);
        time_t tb = entry_time(b, opt);

        if (ta > tb)
            result = -1;
        else if (ta < tb)
            result = 1;
        else {
            long na = entry_nsec(a, opt);
            long nb = entry_nsec(b, opt);

            if (na > nb)
                result = -1;
            else if (na < nb)
                result = 1;
        }
    }

    /* Equal sizes or times are ordered by name. */
    if (result == 0) {
        int name_order = strcoll(a->name, b->name);

        if (name_order == 0)
            name_order = strcmp(a->name, b->name);

        result = (name_order > 0) - (name_order < 0);
    }

    return opt->reverse ? -result : result;
}

void sort_entries(Entry *entries, size_t count,
                  const Options *opt)
{
    if (count < 2 || opt->no_sort)
        return;

    sort_options = opt;
    qsort(entries, count, sizeof(*entries), compare_entries);
}
