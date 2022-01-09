load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository", "new_git_repository")

git_repository(
    name = "googletest",
    remote = "https://github.com/google/googletest",
    tag = "release-1.11.0",
)

new_local_repository(
    name = "ncurses",
    path = "/usr/",
    build_file = "ncurses.BUILD",
)

new_git_repository(
    name = "fmt",
    remote = "https://github.com/fmtlib/fmt",
    commit = "6884aab49b1b7fc6dcba1e27999f1aced0b888be",
    shallow_since = "1641501513 -0800",
    build_file = "//:fmt.BUILD",
)

new_local_repository(
    name = "caf",
    path = "/usr/local", 
    build_file = "caf.BUILD",
)
