<?hh

// Test that COW makes materializing an ImmVector an O(1) operation.

function main() {
  $v = Vector {};
  for ($i = 0; $i < 1000000; $i++) $v[] = 42;
  for ($i = 0; $i < 10000; $i++) $v->toImmVector();

  var_dump($v[0]);
}

main();
