#!/bin/bash
set -euo pipefail
IFS=$'\n\t'

echo "Generating test data"

function generate() {
  local it=$1
  local sym=$2
  local filename=$3
  if [ ! -f $filename ]; then
    echo "Generating $it rows across $sym syms"
    bazel-bin/match-engine-test/test_order testOrderGenerator $it $sym > $filename
  fi
}

generate 100 5 "sample-input/orders-100-5.txt"
generate 1000 50 "sample-input/orders-1k-50.txt"
generate 10000 50 "sample-input/orders-10k-50.txt"
generate 100000 500 "sample-input/orders-10k-500.txt"
generate 1000000 5 "sample-input/orders-1m-5.txt"

generate 1000000 25 "sample-input/orders-1m-25.txt"
generate 1000000 50 "sample-input/orders-1m-50.txt"
generate 1000000 500 "sample-input/orders-1m-500.txt"
generate 1000000 5000 "sample-input/orders-1m-5k.txt"
generate 10000000 5000 "sample-input/orders-10m-5k.txt"

generate 100000000 5 "sample-input/orders-100m-5.txt"
generate 100000000 5000 "sample-input/orders-100m-5k.txt"
