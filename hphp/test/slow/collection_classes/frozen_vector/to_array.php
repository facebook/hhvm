<?hh

// Test casting ImmVector to array.

function main() :mixed{
  var_dump(varray(ImmVector {}));
  var_dump(varray(ImmVector {1, 2, 3}));
}


<<__EntryPoint>>
function main_to_array() :mixed{
main();
}
