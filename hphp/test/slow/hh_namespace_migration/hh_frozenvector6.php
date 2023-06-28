<?hh

// Test that ImmVector can be accessed through its fully-qualified name.

function main() :mixed{
  $s = HH\ImmVector { 1, 2, 3 };
  $s2 = \HH\ImmVector { 4, 5 };
  var_dump($s->count());
  var_dump($s2->count());
}


<<__EntryPoint>>
function main_hh_frozenvector6() :mixed{
main();
}
