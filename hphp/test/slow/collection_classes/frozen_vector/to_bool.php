<?hh

// Test casting a FixedVector to bool.

function main() {
  var_dump((bool) FixedVector {1, 2, 3});
  var_dump((bool) FixedVector {});
}

main();
