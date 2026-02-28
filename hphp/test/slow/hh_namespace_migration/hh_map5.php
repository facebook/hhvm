<?hh

// Test that Map is put in the HH namespace.

namespace HH;

function main() :mixed{
  $m = Map {1 => 1, 2 => 2, 3 => 3}; // Should work.
  \var_dump($m->count());
}


<<__EntryPoint>>
function main_hh_map5() :mixed{
main();
}
