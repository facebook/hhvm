<?hh

// Test that trying to set an element of a FrozenSet through an index
// triggers an error.

function main() {
  $fs = FrozenSet {1, 2, 3};
  $fs[0] = 0;
}

main();
