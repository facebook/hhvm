<?hh

// Test array_key_exists() for ImmVector.

function main() :mixed{
  $fv = ImmVector {1, 2, 3};
  var_dump(array_key_exists(0, $fv));
  var_dump(array_key_exists(3, $fv));
}


<<__EntryPoint>>
function main_array_key_exists() :mixed{
main();
}
