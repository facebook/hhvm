<?hh

// Test casting FixedVector to array.

function main() {
  var_dump((array) FixedVector {});
  var_dump((array) FixedVector {1, 2, 3});
}

main();
