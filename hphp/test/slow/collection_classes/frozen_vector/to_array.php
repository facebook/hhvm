<?hh

// Test casting FrozenVector to array.

function main() {
  var_dump((array) FrozenVector {});
  var_dump((array) FrozenVector {1, 2, 3});
}

main();
