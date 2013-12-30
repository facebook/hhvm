<?hh

// Test that HH\FrozenSet is not "auto-imported" in a named
// namespace.

namespace Test;

function main() {
  $s = new FrozenSet(); // Should cause an error.
}

main();
