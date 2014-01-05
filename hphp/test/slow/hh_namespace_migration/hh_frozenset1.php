<?hh

// Test that in the top-level namespace HH\FrozenSet can be
// accessed as FrozenSet.

function main() {
  $s = FrozenSet {1, 2, 3};
  var_dump($s->isEmpty());
}

main();
