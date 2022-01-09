
load("@rules_cc//cc:defs.bzl", "cc_library", "cc_binary")
load("@//:global_copts.bzl", "global_copts")

package(features = ["-default_compile_flags"])

cc_library(
    name = "lib",
    srcs = glob(["**/*.cc"]),
    hdrs = glob(["**/*.h"]),
    copts = global_copts(),
    include_prefix = "darkpool",
    deps = [
        "@fmt//:main",
        "//common:lib",
    ],
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "main",
    copts = global_copts(),
    linkopts = ["-pthread"],
    deps = [
        "//darkpool:lib",
    ],
    visibility = ["//visibility:public"],

)