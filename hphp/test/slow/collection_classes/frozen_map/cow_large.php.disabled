<?hh

// Test that COW makes materializing an ImmMap an O(1) operation.

function main() {
  $m = Map {};
  for ($i = 0; $i < 1000000; $i++) $m[$i] = 42;
  for ($i = 0; $i < 10000; $i++) $m->toImmMap();

  var_dump($m[0]);
}

main();
