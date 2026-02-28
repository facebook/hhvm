<?hh

// Test that Set can be accessed through its fully-qualified name.

function main() :mixed{
  $s = HH\Set { 1, 2, 3 };
  $s2 = \HH\Set { 4, 5 };
  var_dump($s->count());
  var_dump($s2->count());
}


<<__EntryPoint>>
function main_hh_set6() :mixed{
main();
}
