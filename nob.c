#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#define NOB_EXPERIMENTAL_DELETE_OLD
#include "nob.h"

Cmd cmd = {0};

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    cmd_append(&cmd, "cc");
    cmd_append(&cmd, "-Wall", "-Wextra", "-Wno-unused-function");
    cmd_append(&cmd, "-ggdb");
    cmd_append(&cmd, "-o", "main", "main.c");
    cmd_append(&cmd, "-lffi");

    if (!cmd_run(&cmd)) return 1;

    return 0;
}

