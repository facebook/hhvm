<?hh

// Test creating a ImmVector literal.

function main() {
  $fv = ImmVector {"hello", "world"};
  var_dump($fv->get(0) . ' ' . $fv->get(1));
}


<<__EntryPoint>>
function main_literal_syntax() {
main();
}
