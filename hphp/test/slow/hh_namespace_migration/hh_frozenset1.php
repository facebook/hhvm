<?hh

// Test that in the top-level namespace HH\ImmSet can be
// accessed as ImmSet.

function main() {
  $s = ImmSet {1, 2, 3};
  var_dump($s->isEmpty());
}

main();
