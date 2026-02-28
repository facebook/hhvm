<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

namespace Builtin3;

function test_loaded($files) :mixed{
  foreach ($files as $v) {
    $loaded =
      \__hhvm_intrinsics\is_unit_loaded(__DIR__ . '/' . $v) ? "true" : "false";
    echo "  $v ==> $loaded\n";
  }
}

function prefetch($files) :mixed{
  $f = keyset[];
  foreach ($files as $v) {
    $f[] = __DIR__ . '/' . $v;
  }
  // Advisory and prefetching isn't enabled, so this won't do
  // anything.
  \HH\prefetch_units($f, true);
}

<<__EntryPoint>>
function main() :mixed{
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
