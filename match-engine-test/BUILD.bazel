load("@rules_cc//cc:defs.bzl", "cc_binary")
load("@//:global_copts.bzl", "global_copts")

package(features = ["-default_compile_flags"])

cc_binary(
    name = "test_order",
    srcs = glob([
        "**/*.cc",
        "**/*.h",
    ]),
    copts = global_copts(),
    linkopts = [
        "-pthread",
        "-lstdc++fs",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//common:lib",
    ],
)
