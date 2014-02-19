<?hh

// Test that the var_dump() of a FixedSet is meaningful.

function main() {
  var_dump(new FixedSet(Vector {1, 2, 3}));
}

main();
