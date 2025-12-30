/**
 * Build system: 2AM Builder
 * Original source: https://github.com/kunyukzz/2am-builder.git
 * License: MIT (included in 2am-builder.h)
 */

#define TWO_AM_BUILD_IMPL
#include "2am-builder.h"

static void compiler_config(void)
{
    AM_SET_COMPILER("clang", "c99");
    AM_SET_COMPILER_WARN("-Wall, -Wextra, -Wpedantic, -Werror, "
                         "-Wconversion, -Wsign-conversion, "
                         "-Wshadow, -Wstrict-prototypes, "
                         "-Wunused-parameter");

    AM_ADD_INCLUDE("engine/src");
}

static void source_files(const char *source_dir, const char *build_dir,
                         const char *target)
{
    AM_SET_TARGET_DIRS("bin", build_dir);
    AM_SET_SOURCE_ALL(source_dir);
    AM_SET_TARGET_NAME(target);
}

int main(void)
{
    AM_INIT();

    // engine compile setup
    compiler_config();
    source_files("engine/src", "build/engine", "twoam");

    AM_SET_FLAGS("-g, -fPIC, -DAM2_DEBUG, -DAM2_CORE, -fvisibility=hidden");
    AM_USE_LIB("X11, xcb, X11-xcb, xkbcommon");

    AM_BUILD(BUILD_SHARED, true);
    AM_GEN_DATABASE();

    AM_RESET();

    // testbed compile setup
    compiler_config();
    source_files("testbed/src", "build/testbed", "testbed");

    AM_SET_FLAGS("-g");
    AM_USE_PREBUILT_LIB("twoam", "bin");
    AM_ADD_LINKER_FLAGS("-Wl, -rpath, .");

    AM_BUILD(BUILD_EXE, false);

    AM_RESET();

    return 0;
}
