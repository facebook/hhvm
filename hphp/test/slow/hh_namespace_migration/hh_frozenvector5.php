<?hh

// Test that ImmVector is put in the HH namespace.

namespace HH;

function main() :mixed{
  $s = ImmVector {1, 2, 3}; // Should work.
  \var_dump($s->count());
}


<<__EntryPoint>>
function main_hh_frozenvector5() :mixed{
main();
}
