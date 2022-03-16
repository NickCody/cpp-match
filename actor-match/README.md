# Actor Match Engine


To run:

    bazel run //actor-match:main < sample-input/test-orders.txt

To benchmark:

    time bazel run //actor-match:main < sample-input/orders-1m-5k.txt > /dev/null
