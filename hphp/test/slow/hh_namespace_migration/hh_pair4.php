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

function main() :mixed{
  $custom_set = new Pair();
}


<<__EntryPoint>>
function main_hh_pair4() :mixed{
main();
}
