<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function test_case(string $serialized): void {
  printf("=== %s ===\n", $serialized);
  $unserialized = unserialize(
    $serialized,
    dict['mark_legacy_arrays' => true],
  );
  printf("%s\n", HH\is_array_marked_legacy($unserialized) ? 'true' : 'false');
  var_dump($unserialized);
}

<<__EntryPoint>>
function main(): void {
  // these should get marked
  test_case('a:0:{}');
  test_case('y:0:{}');
  test_case('Y:0:{}');
  test_case('x:0:{}');
  test_case('X:0:{}');
  // these should not
  test_case('v:0:{}');
  test_case('D:0:{}');
  test_case('k:0:{}');
}
