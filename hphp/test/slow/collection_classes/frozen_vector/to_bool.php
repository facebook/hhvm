<?hh

// Test casting a FrozenVector to bool.

function main() {
  var_dump((bool) FrozenVector {1, 2, 3});
  var_dump((bool) FrozenVector {});
}

main();
