<?hh

// Test that Pair is put in the HH namespace.

namespace HH;

function main() {
  $s = Pair {1, 2}; // Should work.
  var_dump($s->count());
}

main();
