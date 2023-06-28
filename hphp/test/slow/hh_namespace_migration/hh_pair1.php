<?hh

// Test that in the top-level namespace HH\Pair can be
// accessed as Pair.

function main() :mixed{
  $s = Pair {1, 2};
  \var_dump($s->isEmpty());
}


<<__EntryPoint>>
function main_hh_pair1() :mixed{
main();
}
