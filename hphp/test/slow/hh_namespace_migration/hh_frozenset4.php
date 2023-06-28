<?hh

// Test that we can define our own custom ImmSet class
// as long as we're not in the top-level namespace.

namespace Test;

class ImmSet {
  public function __construct() {
    echo "Custom ImmSet\n";
  }
}

function main() :mixed{
  $custom_set = new ImmSet();
}


<<__EntryPoint>>
function main_hh_frozenset4() :mixed{
main();
}
