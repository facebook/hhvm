<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function test_loaded() {
  $files = vec[
    'prefetch-worker.php',
    'A.inc',
    'B.inc',
    'C.inc',
    'prefetch-worker.inc'
  ];
  foreach ($files as $v) {
    $loaded =
      __hhvm_intrinsics\is_unit_loaded(__DIR__ . '/' . $v) ? "true" : "false";
    echo "  $v ==> $loaded\n";
  }
}

<<__EntryPoint>>
function main() {
  HH\autoload_set_paths(
    dict[
      'class' => dict[
        'a' => 'A.inc',
        'b' => 'B.inc',
        'c' => 'C.inc',
        'dummy' => 'prefetch-worker.inc'
      ],
    ],
    __DIR__.'/'
  );

  echo "Before:\n";
  test_loaded();
  new Dummy();
  __hhvm_intrinsics\drain_unit_prefetcher();
  echo "After:\n";
  test_loaded();
}
