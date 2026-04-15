#define NOB_IMPLEMENTATION
#include "nob.h"

#define BUILD_DIR "./build/"
#define SRC_DIR "./src/"

static const char *srcs[] = {SRC_DIR "buffer.c", SRC_DIR "main.c"};

int main(int argc, char **argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);

    if (!nob_mkdir_if_not_exists(BUILD_DIR))
        return 1;

    Nob_Cmd cmd = {0};
    nob_cc(&cmd);
    nob_cc_flags(&cmd);
    nob_cmd_append(&cmd, "-D_DEFAULT_SOURCE", "-D_XOPEN_SOURCE=600",
                   "-lncursesw", "-lm");
    nob_cc_output(&cmd, BUILD_DIR "ped");
    for (size_t i = 0; i < NOB_ARRAY_LEN(srcs); ++i) {
        nob_cc_inputs(&cmd, srcs[i]);
    }

    if (!cmd_run(&cmd))
        return 1;

    return 0;
}
