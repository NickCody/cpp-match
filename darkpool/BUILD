load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")
load("@//:global_copts.bzl", "global_copts")

package(features = ["-default_compile_flags"])

cc_library(
    name = "lib",
    srcs = glob(["**/*.cc"]),
    hdrs = glob(
        ["**/*.h"],
        exclude = ["main.cc"],
    ),
    copts = global_copts(),
    include_prefix = "darkpool",
    visibility = ["//visibility:public"],
    deps = [
        "//common:lib",
        "@fmt//:main",
    ],
)

cc_binary(
    name = "main",
    srcs = ["main.cc"],
    copts = global_copts(),
    linkopts = ["-pthread"],
    visibility = ["//visibility:public"],
    deps = [
        "//darkpool:lib",
    ],
)
