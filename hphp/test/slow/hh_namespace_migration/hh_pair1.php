<?hh

// Test that in the top-level namespace HH\Pair can be
// accessed as Pair.

function main() {
  $s = Pair {1, 2};
  var_dump($s->isEmpty());
}

main();
