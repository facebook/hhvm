<?hh

// Test creating a ImmVector literal.

function main() :mixed{
  $fv = ImmVector {"hello", "world"};
  var_dump($fv->get(0) . ' ' . $fv->get(1));
}


<<__EntryPoint>>
function main_literal_syntax() :mixed{
main();
}
