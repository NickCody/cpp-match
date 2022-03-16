load("@rules_cc//cc:defs.bzl", "cc_library")
load("@//:global_copts.bzl", "global_copts")

package(features = ["-default_compile_flags"])

cc_library(
    name = "lib",
    srcs = glob(["**/*.cpp"]),
    hdrs = glob(["**/*.h"]),
    copts = global_copts(),
    include_prefix = "common",
    visibility = ["//visibility:public"],
    deps = [],
)
