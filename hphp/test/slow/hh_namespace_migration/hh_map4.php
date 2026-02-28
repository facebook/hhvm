<?hh

// Test that we can define our own custom Map class
// as long as we're not in the top-level namespace.

namespace Test;

class Map {
  public function __construct() {
    echo "Custom Map\n";
  }
}

function main() :mixed{
  $custom_map = new Map();
}


<<__EntryPoint>>
function main_hh_map4() :mixed{
main();
}
