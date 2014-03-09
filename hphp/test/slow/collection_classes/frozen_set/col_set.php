<?hh

// Test that trying to set an element of a ImmSet through an index
// triggers an error.

function main() {
  $fs = ImmSet {1, 2, 3};
  $fs[0] = 0;
}

main();
