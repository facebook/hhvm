<?hh

// Test array-like access.

function main() {
  $fv = FixedVector {1, 2, 3};
  var_dump($fv[0]);
  var_dump($fv[2]);
}

main();
