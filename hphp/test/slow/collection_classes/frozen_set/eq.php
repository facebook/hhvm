<?hh

// Test that FrozenSets can be compared for equality.

function main() {
  $fs1 = new FrozenSet(Vector {1, 2, 3});
  $fs2 = new FrozenSet(Vector {3, 2, 1});
  var_dump($fs1 == $fs2);
}

main();
