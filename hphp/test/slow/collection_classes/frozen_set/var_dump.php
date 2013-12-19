<?hh

// Test that the var_dump() of a FrozenSet is meaningful.

function main() {
  var_dump(new FrozenSet(Vector {1, 2, 3}));
}

main();
