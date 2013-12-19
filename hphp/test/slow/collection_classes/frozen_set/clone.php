<?hh

// Test that we can clone a FrozenSet.

function main() {
  $fs = FrozenSet {1, 2, 3};
  $clone = clone $fs;
  var_dump($clone == $fs);
}

main();
