<?hh

// Test that ImmSet can be accessed through its fully-qualified name.

function main() :mixed{
  $s = HH\ImmSet { 1, 2, 3 };
  $s2 = \HH\ImmSet { 4, 5 };
  var_dump($s->count());
  var_dump($s2->count());
}


<<__EntryPoint>>
function main_hh_frozenset6() :mixed{
main();
}
