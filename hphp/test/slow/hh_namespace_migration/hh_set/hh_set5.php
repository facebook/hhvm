<?hh

// Test that Set is put in the HH namespace.

namespace HH;

function main() {
  $s = Set {1, 2, 3}; // Should work.
  var_dump($s->count());
}

main();
