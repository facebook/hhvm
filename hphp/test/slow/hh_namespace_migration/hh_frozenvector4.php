<?hh

// Test that we can define our own custom FixedVector class
// as long as we're not in the top-level namespace.

namespace Test;

class FixedVector {
  public function __construct() {
    echo "Custom FixedVector\n";
  }
}

function main() {
  $custom_set = new FixedVector();
}

main();
