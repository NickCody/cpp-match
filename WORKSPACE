load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository", "new_git_repository")

git_repository(
    name = "googletest",
    remote = "https://github.com/google/googletest",
    tag = "release-1.11.0",
)

new_local_repository(
    name = "ncurses",
    build_file = "ncurses.BUILD",
    path = "/usr/",
)

new_git_repository(
    name = "fmt",
    build_file = "//:fmt.BUILD",
    commit = "6884aab49b1b7fc6dcba1e27999f1aced0b888be",
    remote = "https://github.com/fmtlib/fmt",
    shallow_since = "1641501513 -0800",
)

new_local_repository(
    name = "caf",
    build_file = "caf.BUILD",
    path = "/usr/local",
)

new_local_repository(
    name = "zeromq",
    build_file = "zeromq.BUILD",
    path = "/usr",
)

new_local_repository(
    name = "eigen3",
    build_file = "eigen3.BUILD",
    path = "/usr/include",
)

new_git_repository(
    name = "cnl",
    build_file = "//:cnl.BUILD",
    commit = "3ef9b0e224f135dbfed9d210fa8bdf53367b18ff",
    remote = "https://github.com/johnmcfarlane/cnl",
    shallow_since = "1659776808 +0100",
)
