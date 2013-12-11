<?hh

// Test that trying to index a FrozenSet array-style triggers an error.

function main() {
  $fs = FrozenSet {1, 2, 3};
  $x = $fs[0];
}

main();
