#include "myls.h"

#include <locale.h>
#include <stdlib.h>

/* Shared exit status: nonzero if any operation fails. */
int exit_status = 0;

int main(int argc, char **argv)
{
    Options opt;
    int first;
    char *default_paths[] = { "." };

    setlocale(LC_ALL, "");

    first = parse_options(argc, argv, &opt);
    if (first < 0)
        return EXIT_FAILURE;

    if (first == argc)
        list_operands(1, default_paths, &opt);
    else
        list_operands(argc - first, argv + first, &opt);

    return exit_status;
}
