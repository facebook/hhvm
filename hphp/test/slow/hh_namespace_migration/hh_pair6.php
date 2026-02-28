<?hh

// Test that Pair can be accessed through its fully-qualified name.

function main() :mixed{
  $s = HH\Pair { 1, 2 };
  $s2 = \HH\Pair { 4, 5 };
  var_dump($s->count());
  var_dump($s2->count());
}


<<__EntryPoint>>
function main_hh_pair6() :mixed{
main();
}
