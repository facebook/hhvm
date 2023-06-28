<?hh

// Test that in the top-level namespace HH\ImmSet can be
// accessed as ImmSet.

function main() :mixed{
  $s = ImmSet {1, 2, 3};
  \var_dump($s->isEmpty());
}


<<__EntryPoint>>
function main_hh_frozenset1() :mixed{
main();
}
