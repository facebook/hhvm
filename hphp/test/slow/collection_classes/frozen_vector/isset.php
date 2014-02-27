<?hh

// Test isset() with ImmVector.

function main() {
  $fv = ImmVector {1, 2, 3};
  var_dump(isset($fv[0]));
  var_dump(isset($fv[2]));
  var_dump(isset($fv[3]));
}

main();
