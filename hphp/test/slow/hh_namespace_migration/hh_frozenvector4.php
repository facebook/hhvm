<?hh

// Test that we can define our own custom ImmVector class
// as long as we're not in the top-level namespace.

namespace Test;

class ImmVector {
  public function __construct() {
    echo "Custom ImmVector\n";
  }
}

function main() {
  $custom_set = new ImmVector();
}

main();
