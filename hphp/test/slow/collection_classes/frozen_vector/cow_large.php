<?hh

// Test that COW makes materializing a FV an O(1) operation.

function main() {
  $v = Vector {};
  for ($i = 0; $i < 100000; $i++) $v[] = 42;
  for ($i = 0; $i < 1000; $i++) $v->toFrozenVector();

  var_dump($v[0]);
}

main();
