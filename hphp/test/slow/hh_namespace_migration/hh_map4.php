<?hh

// Test that we can define our own custom Map class
// as long as we're not in the top-level namespace.

namespace Test;

class Map {
  public function __construct() {
    echo "Custom Map\n";
  }
}

function main() {
  $custom_map = new Map();
}

main();
