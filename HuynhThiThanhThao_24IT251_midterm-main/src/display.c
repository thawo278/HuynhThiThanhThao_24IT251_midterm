#include "myls.h"

#include <sys/types.h>
#include <ctype.h>
#include <errno.h>
#include <grp.h>
#include <inttypes.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>
#include <wctype.h>

/* Print filenames, replacing non-printable characters in -q mode. */
static void print_name(const char *text, const Options *opt)
{
    mbstate_t state;
    size_t remaining = strlen(text);

    if (!opt->replace_nonprint) {
        fputs(text, stdout);
        return;
    }

    memset(&state, 0, sizeof(state));

    while (remaining > 0) {
        wchar_t wc;
        size_t length = mbrtowc(&wc, text, remaining, &state);

        if (length == (size_t)-1 || length == (size_t)-2) {
            putchar('?');
            memset(&state, 0, sizeof(state));
            text++;
            remaining--;
            continue;
        }

        if (length == 0)
            break;

        if (iswprint(wc))
            fwrite(text, 1, length, stdout);
        else
            putchar('?');

        text += length;
        remaining -= length;
    }
}

static char file_type(mode_t mode)
{
    if (S_ISDIR(mode))
        return 'd';
    if (S_ISLNK(mode))
        return 'l';
    if (S_ISCHR(mode))
        return 'c';
    if (S_ISBLK(mode))
        return 'b';
    if (S_ISFIFO(mode))
        return 'p';
    if (S_ISSOCK(mode))
        return 's';

#ifdef S_ISWHT
    if (S_ISWHT(mode))
        return 'w';
#endif

    return '-';
}

/* Format file type and rwx permissions, including special bits. */
static void format_mode(mode_t mode, char result[11])
{
    static const mode_t bits[] = {
        S_IRUSR, S_IWUSR, S_IXUSR,
        S_IRGRP, S_IWGRP, S_IXGRP,
        S_IROTH, S_IWOTH, S_IXOTH
    };
    const char *letters = "rwxrwxrwx";
    int i;

    result[0] = file_type(mode);

    for (i = 0; i < 9; i++)
        result[i + 1] = (mode & bits[i]) ? letters[i] : '-';

    if (mode & S_ISUID)
        result[3] = (mode & S_IXUSR) ? 's' : 'S';

    if (mode & S_ISGID)
        result[6] = (mode & S_IXGRP) ? 's' : 'S';

    if (mode & S_ISVTX)
        result[9] = (mode & S_IXOTH) ? 't' : 'T';

    result[10] = '\0';
}

static void print_human(long double bytes)
{
    static const char units[] = "BKMGTPE";
    size_t unit = 0;

    while (bytes >= 1024 && unit < sizeof(units) - 2) {
        bytes /= 1024;
        unit++;
    }

    if (unit == 0 || bytes >= 10)
        printf("%.0Lf%c", bytes, units[unit]);
    else
        printf("%.1Lf%c", bytes, units[unit]);
}

static uintmax_t allocated_blocks(const Entry *entry)
{
    if (entry->st.st_blocks < 0)
        return 0;

    return (uintmax_t)entry->st.st_blocks;
}

/* st_blocks is measured in 512-byte units. */
static void print_blocks(uintmax_t blocks, const Options *opt)
{
    if (opt->human_readable) {
        print_human((long double)blocks * 512);
    } else {
        uintmax_t bytes;
        uintmax_t units;

        if (blocks > UINTMAX_MAX / 512) {
            fputs("overflow", stdout);
            exit_status = 1;
            return;
        }

        bytes = blocks * 512;
        units = bytes / opt->block_size;

        if (bytes % opt->block_size != 0)
            units++;

        printf("%ju", units);
    }
}

void print_total(const Entry *entries, size_t count,
                 const Options *opt)
{
    uintmax_t total = 0;
    size_t i;

    for (i = 0; i < count; i++) {
        uintmax_t blocks = allocated_blocks(&entries[i]);

        if (blocks > UINTMAX_MAX - total) {
            fprintf(stderr, "myls: block total overflow\n");
            exit_status = 1;
            return;
        }

        total += blocks;
    }

    fputs("total ", stdout);
    print_blocks(total, opt);
    putchar('\n');
}

static void print_owner(uid_t uid, const Options *opt)
{
    struct passwd *owner = NULL;

    if (!opt->numeric_ids)
        owner = getpwuid(uid);

    if (owner != NULL)
        printf("%s ", owner->pw_name);
    else
        printf("%ju ", (uintmax_t)uid);
}

static void print_group(gid_t gid, const Options *opt)
{
    struct group *group = NULL;

    if (!opt->numeric_ids)
        group = getgrgid(gid);

    if (group != NULL)
        printf("%s ", group->gr_name);
    else
        printf("%ju ", (uintmax_t)gid);
}

static void print_timestamp(const Entry *entry, const Options *opt)
{
    time_t value = entry_time(entry, opt);
    struct tm *local = localtime(&value);
    char buffer[80];

    if (local == NULL ||
        strftime(buffer, sizeof(buffer), "%b %e %H:%M", local) == 0) {
        fputs("??? ?? ??:?? ", stdout);
        return;
    }

    printf("%s ", buffer);
}

static void print_suffix(mode_t mode)
{
    if (S_ISDIR(mode))
        putchar('/');
    else if (S_ISLNK(mode))
        putchar('@');
    else if (S_ISSOCK(mode))
        putchar('=');
    else if (S_ISFIFO(mode))
        putchar('|');
#ifdef S_ISWHT
    else if (S_ISWHT(mode))
        putchar('%');
#endif
    else if (mode & (S_IXUSR | S_IXGRP | S_IXOTH))
        putchar('*');
}

static void print_link_target(const Entry *entry, const Options *opt)
{
    size_t capacity = 256;
    char *buffer = NULL;

    for (;;) {
        char *new_buffer;
        ssize_t length;

        new_buffer = realloc(buffer, capacity + 1);

        if (new_buffer == NULL) {
            free(buffer);
            fprintf(stderr, "myls: out of memory\n");
            exit(EXIT_FAILURE);
        }

        buffer = new_buffer;
        length = readlink(entry->path, buffer, capacity);

        if (length == -1) {
            int saved_errno = errno;

            free(buffer);
            fprintf(stderr, "myls: %s: %s\n",
                    entry->path, strerror(saved_errno));
            exit_status = 1;
            return;
        }

        if ((size_t)length < capacity) {
            buffer[length] = '\0';
            fputs(" -> ", stdout);
            print_name(buffer, opt);
            free(buffer);
            return;
        }

        if (capacity > ((size_t)-1 - 1) / 2) {
            free(buffer);
            fprintf(stderr, "myls: link target too long\n");
            exit_status = 1;
            return;
        }

        capacity *= 2;
    }
}

void print_entry(const Entry *entry, const Options *opt)
{
    const struct stat *st = &entry->st;

    if (opt->show_inode)
        printf("%ju ", (uintmax_t)st->st_ino);

    if (opt->show_blocks) {
        print_blocks(allocated_blocks(entry), opt);
        putchar(' ');
    }

    if (opt->long_format) {
        char mode[11];

        format_mode(st->st_mode, mode);
        printf("%s %ju ", mode, (uintmax_t)st->st_nlink);

        print_owner(st->st_uid, opt);
        print_group(st->st_gid, opt);

        if (S_ISCHR(st->st_mode) || S_ISBLK(st->st_mode)) {
            printf("%ju, %ju",
                   (uintmax_t)major(st->st_rdev),
                   (uintmax_t)minor(st->st_rdev));
        } else if (opt->human_readable) {
            print_human((long double)st->st_size);
        } else {
            printf("%jd", (intmax_t)st->st_size);
        }

        putchar(' ');
        print_timestamp(entry, opt);
    }

    print_name(entry->name, opt);

    if (opt->classify)
        print_suffix(st->st_mode);

    if (opt->long_format && S_ISLNK(st->st_mode))
        print_link_target(entry, opt);

    putchar('\n');
}
