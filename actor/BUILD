load("@rules_cc//cc:defs.bzl", "cc_binary")
load("@//:global_copts.bzl", "global_copts")

package(features = ["-default_compile_flags"])

cc_binary(
    name = "actor",
    srcs = ["actor.cpp"],
    copts = global_copts(),
    linkopts = [
        "-pthread",
        "-lstdc++fs",
    ],
    deps = [
        "@caf//:main",
    ],
)

cc_binary(
    name = "class",
    srcs = ["class.cpp"],
    copts = global_copts(),
    linkopts = [
        "-pthread",
        "-lstdc++fs",
    ],
    deps = [
        "@caf//:main",
    ],
)

cc_binary(
    name = "class-perf",
    srcs = ["class-perf.cpp"],
    copts = global_copts(),
    linkopts = [
        "-pthread",
        "-lstdc++fs",
    ],
    deps = [
        "//common:lib",
        "@caf//:main",
        "@fmt//:main",
    ],
)
