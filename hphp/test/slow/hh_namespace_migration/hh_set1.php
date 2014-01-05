<?hh

// Test that in the top-level namespace HH\Set can be
// accessed as Set.

function main() {
  $s = Set {1, 2, 3};
  var_dump($s->isEmpty());
}

main();
