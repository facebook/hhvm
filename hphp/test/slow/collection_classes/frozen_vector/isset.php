<?hh

// Test isset() with FrozenVector.

function main() {
  $fv = FrozenVector {1, 2, 3};
  var_dump(isset($fv[0]));
  var_dump(isset($fv[2]));
  var_dump(isset($fv[3]));
}

main();
