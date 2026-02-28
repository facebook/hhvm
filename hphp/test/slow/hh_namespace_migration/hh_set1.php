<?hh

// Test that in the top-level namespace HH\Set can be
// accessed as Set.

function main() :mixed{
  $s = Set {1, 2, 3};
  \var_dump($s->isEmpty());
}


<<__EntryPoint>>
function main_hh_set1() :mixed{
main();
}
