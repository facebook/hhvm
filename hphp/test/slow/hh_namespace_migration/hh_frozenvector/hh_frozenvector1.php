<?hh

// Test that in the top-level namespace HH\FrozenVector can be
// accessed as FrozenVector.

function main() {
  $s = FrozenVector {1, 2, 3};
  var_dump($s->isEmpty());
}

main();
