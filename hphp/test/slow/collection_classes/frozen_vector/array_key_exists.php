<?hh

// Test array_key_exists() for FrozenVector.

function main() {
  $fv = FrozenVector {1, 2, 3};
  var_dump(array_key_exists(0, $fv));
  var_dump(array_key_exists(3, $fv));
}

main();
