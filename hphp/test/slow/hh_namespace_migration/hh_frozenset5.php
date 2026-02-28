<?hh

// Test that ImmSet is put in the HH namespace.

namespace HH;

function main() :mixed{
  $s = ImmSet {1, 2, 3}; // Should work.
  \var_dump($s->count());
}


<<__EntryPoint>>
function main_hh_frozenset5() :mixed{
main();
}
