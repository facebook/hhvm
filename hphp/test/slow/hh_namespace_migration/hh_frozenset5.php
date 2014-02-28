<?hh

// Test that ImmSet is put in the HH namespace.

namespace HH;

function main() {
  $s = ImmSet {1, 2, 3}; // Should work.
  var_dump($s->count());
}

main();
