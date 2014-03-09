<?hh

// Test that trying to index a ImmSet array-style triggers an error.

function main() {
  $fs = ImmSet {1, 2, 3};
  $x = $fs[0];
}

main();
