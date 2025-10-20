#define NOB_IMPLEMENTATION

#include <nob/nob.h>

#define SRC_DIR "src/"
#define INC_DIR "include/"
#define BUILD_DIR "build/"
#define CC "clang"
#define BIN BUILD_DIR "shire"

static const char *srcs[] = {
    SRC_DIR "main.c",
    SRC_DIR "buffer.c",
    SRC_DIR "modes.c",
    SRC_DIR "utils.c",
};

int main(int argc, char **argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);
    if (!nob_mkdir_if_not_exists(BUILD_DIR))
        return 1;

    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, CC, "-I" INC_DIR);

    nob_cmd_append(&cmd,
                   "-Wall",
                   "-Wextra",
                   "-pedantic",
                   "-D_FORTIFY_SOURCE",
                   "-fstack-protector-strong",
                   "-O2");

    for (size_t i = 0; i < NOB_ARRAY_LEN(srcs); i++) {
        nob_cmd_append(&cmd, srcs[i]);
    }

    nob_cmd_append(&cmd, "-o", BIN);

    nob_cmd_append(&cmd, "-lncurses");

    if (!nob_cmd_run_sync(cmd))
        return 1;

    nob_log(NOB_INFO, "Build successful: %s", BIN);

    return 0;
}
