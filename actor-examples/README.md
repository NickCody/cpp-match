# Actor Examples


# Benchmarks

    bazel run //actor-examples:class-perf -- --config-file=/workspaces/trade/actor-examples/class-perf.caf | column -t -s,

Sort by method

    bazel run //actor-examples:class-perf -- --config-file=/workspaces/trade/actor-examples/class-perf.caf | sort -t, --key=7 | column -t -s,