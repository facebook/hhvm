<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

namespace PrefetchWorkerOff;

function test_loaded() :mixed{
  $files = vec[
    'prefetch-worker-off.php',
    'A.inc',
    'B.inc',
    'C.inc',
    'prefetch-worker.inc'
  ];
  foreach ($files as $v) {
    $loaded =
      \__hhvm_intrinsics\is_unit_loaded(__DIR__ . '/' . $v) ? "true" : "false";
    echo "  $v ==> $loaded\n";
  }
}

<<__EntryPoint>>
function main() :mixed{
  echo "Before:\n";
  test_loaded();
  new \Dummy();
  \__hhvm_intrinsics\drain_unit_prefetcher();
  echo "After:\n";
  test_loaded();
}
