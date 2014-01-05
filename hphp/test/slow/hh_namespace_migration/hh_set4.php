<?hh

// Test that we can define our own custom Set class
// as long as we're not in the top-level namespace.

namespace Test;

class Set {
  public function __construct() {
    echo "Custom Set\n";
  }
}

function main() {
  $custom_set = new Set();
}

main();
