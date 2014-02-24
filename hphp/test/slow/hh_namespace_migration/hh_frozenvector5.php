<?hh

// Test that FixedVector is put in the HH namespace.

namespace HH;

function main() {
  $s = FixedVector {1, 2, 3}; // Should work.
  var_dump($s->count());
}

main();
