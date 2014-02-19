<?hh

// Test that we can define our own custom FixedSet class
// as long as we're not in the top-level namespace.

namespace Test;

class FixedSet {
  public function __construct() {
    echo "Custom FixedSet\n";
  }
}

function main() {
  $custom_set = new FixedSet();
}

main();
