# Actor Match Engine


To run:

    bazel run //actor-match:main < sample-input/test-orders.txt

To benchmark:

    time bazel run //actor-match:main < sample-input/orders-1m-5k.txt > /dev/null
    time bazel-bin/actor-match/main sample-input/orders-1m-5k.txt > /dev/null

Soecify custom config file:

    time bazel run //actor-match:main -- --config-file=/workspaces/trade/actor-match/match_config.caf < sample-input/orders-1m-5k.txt > /dev/null
    
    bazel build //actor-match:main
    time bazel-bin/actor-match/main --config-file=/workspaces/trade/actor-match/match_config.caf < sample-input/orders-1m-5.txt > /dev/null
    time bazel-bin/actor-match/main --config-file=/workspaces/trade/actor-match/match_config.caf < sample-input/orders-1m-50.txt > /dev/null
    time bazel-bin/actor-match/main --config-file=/workspaces/trade/actor-match/match_config.caf < sample-input/orders-1m-500.txt > /dev/null
    time bazel-bin/actor-match/main --config-file=/workspaces/trade/actor-match/match_config.caf < sample-input/orders-1m-5k.txt > /dev/null

Non-Actor Benchmark:

    time bazel run //match-engine:main -- 10 16 < sample-input/orders-1m-5k.txt > /dev/null
    time bazel run //match-engine:main -- 100 0 < sample-input/orders-1m-5k.txt > /dev/null

NOTE:

- 10 is batch size
- 16 is concurrency (threads)

Billion-row Test

    bazel run //match-engine-test:test_order testOrderGenerator 1000000000 20 | bazel run //actor-match:main -- --config-file=/workspaces/trade/actor-match/match_config.caf 1000000 > /dev/null

    bazel-bin/match-engine-test/test_order 1000000000 20 | bazel-bin/actor-match/main --config-file=actor-match/match_config.caf 1000000 > /dev/null
