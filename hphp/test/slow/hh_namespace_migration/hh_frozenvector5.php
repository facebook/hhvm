<?hh

// Test that ImmVector is put in the HH namespace.

namespace HH;

function main() {
  $s = ImmVector {1, 2, 3}; // Should work.
  var_dump($s->count());
}

main();
