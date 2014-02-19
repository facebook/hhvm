<?hh

// Test that we can clone a FixedSet.

function main() {
  $fs = FixedSet {1, 2, 3};
  $clone = clone $fs;
  var_dump($clone == $fs);
}

main();
