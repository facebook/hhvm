<?hh

// Test that in the top-level namespace HH\ImmVector can be
// accessed as ImmVector.

function main() {
  $s = ImmVector {1, 2, 3};
  var_dump($s->isEmpty());
}

main();
