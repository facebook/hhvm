<?hh

// Test that the var_dump() of a ImmSet is meaningful.

function main() {
  var_dump(new ImmSet(Vector {1, 2, 3}));
}

main();
