<?hh

// Test that HH\FrozenVector is not "auto-imported" in a named
// namespace.

namespace Test;

function main() {
  $s = new FrozenVector(); // Should cause an error.
}

main();
