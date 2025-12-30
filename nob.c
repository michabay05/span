#define NOB_IMPLEMENTATION
#include "nob.h"

#define SRCDIR "src/"
#define OBJDIR "obj/"
#define BINARY "span.bin"
#define VENDORDIR "vendor/"
#define VENDOR_INCDIR VENDORDIR"include"
#define VENDOR_LIBDIR VENDORDIR"lib"
#define CC "gcc"

static Nob_Cmd cmd = {0};

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    nob_shift_args(&argc, &argv);
    bool run_bin = false;
    if (argc > 0) {
        if (!strncmp(argv[0], "run", 3)) {
            run_bin = true;
        } else {
            nob_log(NOB_ERROR, "Wrong usage. Will document at some point...");
            return 1;
        }
    }

    nob_cc(&cmd);
    nob_cc_flags(&cmd);
    nob_cc_output(&cmd, "span.bin");
    nob_cmd_append(&cmd, "-Ivendor/include", "-I.");
    nob_cc_inputs(&cmd, "src/span.c");
    nob_cmd_append(&cmd, "-Lvendor/lib");
    nob_cmd_append(&cmd, "-l:libraylib.a");
    nob_cmd_append(&cmd, "-l:libumka.a");
    nob_cmd_append(&cmd, "-lm");
    if (!nob_cmd_run(&cmd)) return 1;

    if (run_bin) {
        cmd.count = 0;
        nob_cmd_append(&cmd, "./"BINARY);
        if (!nob_cmd_run(&cmd)) return 1;
    }
    return 0;
}
