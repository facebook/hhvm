<?hh

// Test array-like access.

function main() {
  $fv = ImmVector {1, 2, 3};
  var_dump($fv[0]);
  var_dump($fv[2]);
}


<<__EntryPoint>>
function main_array_access() {
main();
}
