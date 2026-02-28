<?hh

// Test that we can define our own custom Set class
// as long as we're not in the top-level namespace.

namespace Test;

class Set {
  public function __construct() {
    echo "Custom Set\n";
  }
}

function main() :mixed{
  $custom_set = new Set();
}


<<__EntryPoint>>
function main_hh_set4() :mixed{
main();
}
