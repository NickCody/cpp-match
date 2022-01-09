load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "main",
    srcs = [
        "include/fmt/args.h",
        "include/fmt/chrono.h",
        "include/fmt/color.h",
        "include/fmt/compile.h",
        "include/fmt/core.h",
        "include/fmt/format.h",
        "include/fmt/format-inl.h",
        "include/fmt/locale.h",
        "include/fmt/os.h",
        "include/fmt/ostream.h",
        "include/fmt/printf.h",
        "include/fmt/ranges.h",
        "include/fmt/xchar.h",
        "src/format.cc",
        "src/os.cc",
    ],
    hdrs = [
        "include/fmt/core.h",
        "include/fmt/format.h",
    ],
    includes = [
        "include",
        "src",
    ],
    strip_include_prefix = "include/fmt/",
    visibility = ["//visibility:public"],
)