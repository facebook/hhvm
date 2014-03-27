<?hh

// Test that we can define our own custom Pair class
// as long as we're not in the top-level namespace.

namespace Test;

class Pair {
  public function __construct() {
    echo "Custom Pair\n";
  }
}

use Test\Pair;

function main() {
  $custom_set = new Pair();
}

main();
