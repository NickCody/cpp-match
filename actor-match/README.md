# Actor Match Engine


To run:

    bazel run //actor-match:main < sample-input/test-orders.txt

To benchmark:

    time bazel run //actor-match:main < sample-input/orders-1m-5k.txt > /dev/null

Soecify custom config file:

    time bazel run //actor-match:main -- --config-file=/workspaces/trade/actor-match/match_config.caf < sample-input/orders-100m-5k.txt > /dev/null

Non-Actor Benchmark:

    time bazel run //match-engine:main -- 10 16 < sample-input/orders-1m-5k.txt > /dev/null