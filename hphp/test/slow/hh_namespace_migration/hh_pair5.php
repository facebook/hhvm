<?hh

// Test that Pair is put in the HH namespace.

namespace HH;

function main() :mixed{
  $s = Pair {1, 2}; // Should work.
  \var_dump($s->count());
}


<<__EntryPoint>>
function main_hh_pair5() :mixed{
main();
}
