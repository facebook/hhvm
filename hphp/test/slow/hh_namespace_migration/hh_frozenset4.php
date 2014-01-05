<?hh

// Test that we can define our own custom FrozenSet class
// as long as we're not in the top-level namespace.

namespace Test;

class FrozenSet {
  public function __construct() {
    echo "Custom FrozenSet\n";
  }
}

function main() {
  $custom_set = new FrozenSet();
}

main();
