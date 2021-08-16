filegroup(
    name = "all_files",
    srcs = glob(
        ["*"]
    ),
)

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "libknv",
    srcs = glob(["*.c", "*.cc"]),
    hdrs = glob(["*.h"]),
    includes = ["."],
    deps = [
    ],
    copts = ["-O3", "-fPIC", "-finline-functions", "-falign-loops", "-Wall", "-pthread", "-fno-strict-aliasing", "-g"],
    linkstatic = True,
    alwayslink = True,
)

