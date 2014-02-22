<?hh

// Test that in the top-level namespace HH\FixedVector can be
// accessed as FixedVector.

function main() {
  $s = FixedVector {1, 2, 3};
  var_dump($s->isEmpty());
}

main();
