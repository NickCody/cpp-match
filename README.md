# trade

A trading system simulator for testing new ideas, libraries, etc.

## actor-samples

    bazel run //actor:actor      - Basic Actor application example
    bazel run //actor:class-perf - Performance tests
    bazel run //actor:class      - Basic example on how to add arbitrary state for an actor

## match-engine

   bazel run //match-engine:main - reads orders from stdin
   bazel run //match-engine-test:test_order command, where command

Where commands are
- all
- testDoubleHistograms
- testIntegerHistograms
- testOrderHeapOrdering
- testOrderRanking
- testOrderGenerator
- testOrderHeapAllocation
- testOrderComparison
- testOrderCreation
- testOrderFromString

## actor-match

Match engine written using actors


## Libraries in play

- C++ 20 / gcc 10.2.1
- [CAF: the C++ Actor Framework](https://github.com/actor-framework/actor-framework)
- [{fmt}](https://fmt.dev/latest/index.html)
- [ncurses](https://tldp.org/HOWTO/NCURSES-Programming-HOWTO/intro.html)
