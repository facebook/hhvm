<?hh

// Test that in the top-level namespace HH\Map can be
// accessed as Map.

function main() :mixed{
  $m = Map {1 => 1, 2 => 2, 3 => 3};
  \var_dump($m->isEmpty());
}


<<__EntryPoint>>
function main_hh_map1() :mixed{
main();
}
