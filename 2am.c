/**
 * Build system: 2AM Builder
 * Original source: https://github.com/kunyukzz/2am-builder.git
 * License: MIT (included in 2am-builder.h)
 */

#define TWO_AM_BUILD_IMPL
#include "2am-builder.h"

int main(void)
{
    AM_INIT();

    // engine compile setup
    AM_SET_COMPILER("clang", "c99");

    AM_SET_COMPILER_WARN("-Wall, -Wextra, -Wpedantic, -Werror, "
                         "-Wconversion, -Wsign-conversion, "
                         "-Wshadow, -Wstrict-prototypes, "
                         "-Wmissing-prototypes, -Wunused-parameter");

    AM_SET_FLAGS("-g, -fPIC, -DAM2_DEBUG, -DAM2_CORE");
    AM_SET_TARGET_DIRS("bin", "build/engine");
    AM_ADD_INCLUDE("engine/src");
    AM_SET_SOURCE_ALL("engine/src");
    AM_USE_LIB("X11");
    AM_SET_TARGET_NAME("twoam");

    AM_BUILD(BUILD_SHARED, true);
    AM_GEN_DATABASE();

    AM_RESET();

    // testbed compile setup
    AM_SET_COMPILER("clang", "c99");

    AM_SET_COMPILER_WARN("-Wall, -Wextra, -Wpedantic, -Werror, "
                         "-Wconversion, -Wsign-conversion, "
                         "-Wshadow, -Wstrict-prototypes, "
                         "-Wmissing-prototypes, -Wunused-parameter");

    AM_SET_FLAGS("-g");
    AM_SET_TARGET_DIRS("bin", "build/testbed");
    AM_ADD_INCLUDE("engine/src");
    AM_SET_SOURCE_ALL("testbed/src");
    AM_SET_TARGET_NAME("testbed");

    AM_USE_PREBUILT_LIB("twoam", "bin");
    AM_ADD_LINKER_FLAGS("-Wl, -rpath, .");
    AM_BUILD(BUILD_EXE, false);

    AM_RESET();

    return 0;
}
