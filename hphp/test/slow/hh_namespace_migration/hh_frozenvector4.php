<?hh

// Test that we can define our own custom FrozenVector class
// as long as we're not in the top-level namespace.

namespace Test;

class FrozenVector {
  public function __construct() {
    echo "Custom FrozenVector\n";
  }
}

function main() {
  $custom_set = new FrozenVector();
}

main();
