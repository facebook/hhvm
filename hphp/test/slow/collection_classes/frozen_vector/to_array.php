<?hh

// Test casting ImmVector to array.

function main() {
  var_dump((array) ImmVector {});
  var_dump((array) ImmVector {1, 2, 3});
}


<<__EntryPoint>>
function main_to_array() {
main();
}
