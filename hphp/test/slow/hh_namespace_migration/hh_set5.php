<?hh

// Test that Set is put in the HH namespace.

namespace HH;

function main() :mixed{
  $s = Set {1, 2, 3}; // Should work.
  \var_dump($s->count());
}


<<__EntryPoint>>
function main_hh_set5() :mixed{
main();
}
