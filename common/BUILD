
load("@rules_cc//cc:defs.bzl", "cc_library")
load("@//:global_copts.bzl", "global_copts")

package(features = ["-default_compile_flags"])

cc_library(
    name = "lib",
    srcs = glob(["**/*.cpp"]),
    hdrs = glob(["**/*.h"]),
    include_prefix = "common",
    copts = global_copts(),
    deps = [
        "@fmt//:main",
    ],
    visibility = ["//visibility:public"],
)
