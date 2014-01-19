<?hh

// Test that FrozenVector is put in the HH namespace.

namespace HH;

function main() {
  $s = FrozenVector {1, 2, 3}; // Should work.
  var_dump($s->count());
}

main();
