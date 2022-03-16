load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")
load("@//:global_copts.bzl", "global_copts")

package(features = ["-default_compile_flags"])

cc_library(
    name = "lib",
    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.h"]),
    copts = global_copts(),
    visibility = ["//visibility:public"],
    deps = [
        "//common:lib",
    ],
)

cc_binary(
    name = "main",
    copts = global_copts(),
    linkopts = [
        "-pthread",
        "-lstdc++fs",
    ],
    deps = [
        "//actor-match:lib",
        "//common:lib",
        "//match-engine:lib",
        "@caf//:main",
        "@fmt//:main",
        "@ncurses//:main",
    ],
)
