<?hh

// Test that we can define our own custom ImmSet class
// as long as we're not in the top-level namespace.

namespace Test;

class ImmSet {
  public function __construct() {
    echo "Custom ImmSet\n";
  }
}

function main() {
  $custom_set = new ImmSet();
}

main();
