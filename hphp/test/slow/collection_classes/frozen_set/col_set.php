<?hh

// Test that trying to set an element of a FixedSet through an index
// triggers an error.

function main() {
  $fs = FixedSet {1, 2, 3};
  $fs[0] = 0;
}

main();
