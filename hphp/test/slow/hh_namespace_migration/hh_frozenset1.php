<?hh

// Test that in the top-level namespace HH\FixedSet can be
// accessed as FixedSet.

function main() {
  $s = FixedSet {1, 2, 3};
  var_dump($s->isEmpty());
}

main();
