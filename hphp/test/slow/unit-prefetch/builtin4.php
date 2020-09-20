<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function test_loaded($files) {
  foreach ($files as $v) {
    $loaded =
      __hhvm_intrinsics\is_unit_loaded(__DIR__ . '/' . $v) ? "true" : "false";
    echo "  $v ==> $loaded\n";
  }
}

function prefetch($files) {
  $f = keyset[];
  foreach ($files as $v) {
    $f[] = __DIR__ . '/' . $v;
  }
  // Advisory, but prefetching is enabled. Wait for the prefetcher to
  // finish.
  hh\prefetch_units($f, true);
  __hhvm_intrinsics\drain_unit_prefetcher();
}

<<__EntryPoint>>
function main() {
  $files = keyset[
    'A.inc',
    'B.inc',
    'C.inc'
  ];
  echo "Before:\n";
  test_loaded($files);
  prefetch($files);
  echo "After:\n";
  test_loaded($files);
}
