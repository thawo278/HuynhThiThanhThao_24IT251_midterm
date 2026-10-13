#include "myls.h"

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Track ancestors to detect directory cycles. */
typedef struct Ancestor {
    dev_t device;
    ino_t inode;
    const struct Ancestor *parent;
} Ancestor;

static int output_started = 0;

static void report_error(const char *path)
{
    fprintf(stderr, "myls: %s: %s\n", path, strerror(errno));
    exit_status = 1;
}

static void *checked_alloc(void *old, size_t bytes)
{
    void *p = realloc(old, bytes);

    if (p == NULL) {
        fprintf(stderr, "myls: out of memory\n");
        exit(EXIT_FAILURE);
    }

    return p;
}

static char *copy_text(const char *text)
{
    char *result = checked_alloc(NULL, strlen(text) + 1);

    strcpy(result, text);
    return result;
}

static char *join_path(const char *dir, const char *name)
{
    size_t len = strlen(dir);
    int separator = (len > 0 && dir[len - 1] != '/');
    size_t size = len + separator + strlen(name) + 1;
    char *result = checked_alloc(NULL, size);

    snprintf(result, size, "%s%s%s",
             dir, separator ? "/" : "", name);

    return result;
}

static void free_entries(Entry *entries, size_t count)
{
    size_t i;

    for (i = 0; i < count; i++) {
        free(entries[i].name);
        free(entries[i].path);
    }

    free(entries);
}

static void append_entry(Entry **entries, size_t *count,
                         size_t *capacity, Entry entry)
{
    if (*count == *capacity) {
        size_t next = (*capacity == 0) ? 32 : *capacity * 2;

        if (next < *capacity ||
            next > (size_t)-1 / sizeof(**entries)) {
            fprintf(stderr, "myls: too many entries\n");
            exit(EXIT_FAILURE);
        }

        *entries = checked_alloc(*entries,
                                 next * sizeof(**entries));
        *capacity = next;
    }

    (*entries)[*count] = entry;
    (*count)++;
}

static int visible_name(const char *name, const Options *opt)
{
    if (name[0] != '.')
        return 1;

    if (opt->show_all)
        return 1;

    if (opt->almost_all &&
        strcmp(name, ".") != 0 &&
        strcmp(name, "..") != 0)
        return 1;

    return 0;
}

static void print_heading(const char *path, const Options *opt)
{
    const unsigned char *p = (const unsigned char *)path;

    if (output_started)
        putchar('\n');

    while (*p != '\0') {
        /* Keep control characters out of headings in -q mode. */
        if (opt->replace_nonprint && (*p < 32 || *p == 127))
            putchar('?');
        else
            putchar(*p);
        p++;
    }

    puts(":");
    output_started = 1;
}

static void list_directory(const char *path, const Options *opt,
                           int heading, const Ancestor *parent)
{
    DIR *dir;
    struct dirent *item;
    struct stat dir_stat;
    const Ancestor *p;
    Ancestor current;
    Entry *entries = NULL;
    size_t count = 0;
    size_t capacity = 0;
    size_t i;

    dir = opendir(path);
    if (dir == NULL) {
        report_error(path);
        return;
    }

    if (fstat(dirfd(dir), &dir_stat) == -1) {
        report_error(path);
        closedir(dir);
        return;
    }

    for (p = parent; p != NULL; p = p->parent) {
        if (p->device == dir_stat.st_dev &&
            p->inode == dir_stat.st_ino) {
            fprintf(stderr, "myls: %s: directory cycle\n", path);
            exit_status = 1;
            closedir(dir);
            return;
        }
    }

    current.device = dir_stat.st_dev;
    current.inode = dir_stat.st_ino;
    current.parent = parent;

    for (;;) {
        Entry entry;

        errno = 0;
        item = readdir(dir);

        if (item == NULL) {
            if (errno != 0)
                report_error(path);
            break;
        }

        if (!visible_name(item->d_name, opt))
            continue;

        entry.path = join_path(path, item->d_name);

        if (lstat(entry.path, &entry.st) == -1) {
            report_error(entry.path);
            free(entry.path);
            continue;
        }

        entry.name = copy_text(item->d_name);
        append_entry(&entries, &count, &capacity, entry);
    }

    if (closedir(dir) == -1)
        report_error(path);

    sort_entries(entries, count, opt);

    if (heading)
        print_heading(path, opt);

    if (opt->long_format ||
        (opt->show_blocks && opt->terminal_output)) {
        print_total(entries, count, opt);
        output_started = 1;
    }

    for (i = 0; i < count; i++) {
        print_entry(&entries[i], opt);
        output_started = 1;
    }

    if (opt->recursive) {
        for (i = 0; i < count; i++) {
            if (!S_ISDIR(entries[i].st.st_mode))
                continue;

            if (strcmp(entries[i].name, ".") == 0 ||
                strcmp(entries[i].name, "..") == 0)
                continue;

            list_directory(entries[i].path, opt, 1, &current);
        }
    }

    free_entries(entries, count);
}

void list_operands(int count, char **paths, const Options *opt)
{
    Entry *files = NULL;
    Entry *dirs = NULL;
    size_t file_count = 0;
    size_t dir_count = 0;
    size_t file_capacity = 0;
    size_t dir_capacity = 0;
    size_t i;
    int n;

    for (n = 0; n < count; n++) {
        Entry entry;

        if (lstat(paths[n], &entry.st) == -1) {
            report_error(paths[n]);
            continue;
        }

        /*
         * In a plain listing, follow a command-line symlink
         * to a directory. Keep links visible with -d, -l or -F.
         */
        if (S_ISLNK(entry.st.st_mode) &&
            !opt->directory_only &&
            !opt->long_format &&
            !opt->classify) {
            struct stat target;

            if (stat(paths[n], &target) == 0 &&
                S_ISDIR(target.st_mode))
                entry.st = target;
        }

        entry.name = copy_text(paths[n]);
        entry.path = copy_text(paths[n]);

        if (S_ISDIR(entry.st.st_mode) && !opt->directory_only) {
            append_entry(&dirs, &dir_count,
                         &dir_capacity, entry);
        } else {
            append_entry(&files, &file_count,
                         &file_capacity, entry);
        }
    }

    /* File operands are printed before directory operands. */
    sort_entries(files, file_count, opt);
    sort_entries(dirs, dir_count, opt);

    for (i = 0; i < file_count; i++) {
        print_entry(&files[i], opt);
        output_started = 1;
    }

    for (i = 0; i < dir_count; i++) {
        int heading = (count > 1 || opt->recursive);

        list_directory(dirs[i].path, opt, heading, NULL);
    }

    free_entries(files, file_count);
    free_entries(dirs, dir_count);
}
