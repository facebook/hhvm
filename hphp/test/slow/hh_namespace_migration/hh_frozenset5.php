<?hh

// Test that FixedSet is put in the HH namespace.

namespace HH;

function main() {
  $s = FixedSet {1, 2, 3}; // Should work.
  var_dump($s->count());
}

main();
