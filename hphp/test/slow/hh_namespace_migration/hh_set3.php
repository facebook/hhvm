<?hh

// Test that HH\Set is not "auto-imported" in a named
// namespace.

namespace Test;

function main() {
  $s = new Set(); // Should cause an error.
}

main();
