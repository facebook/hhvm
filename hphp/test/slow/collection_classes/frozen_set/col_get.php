<?hh

// Test that trying to index a FixedSet array-style triggers an error.

function main() {
  $fs = FixedSet {1, 2, 3};
  $x = $fs[0];
}

main();
