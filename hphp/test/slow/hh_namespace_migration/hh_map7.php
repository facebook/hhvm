<?hh

// Test that Map behaves properly when mixing namespaced and
// non-namespaced code.

namespace Test {
  // Can have custom class named Map.

  class Map {
    public function __construct() {
      echo "Custom Map\n";
    }
  }

  function foo() {
    $custom_map = new Map();
    var_dump($custom_map instanceof \HH\Map); // False
  }

}

namespace {

  function bar() {
    $builtin_map = new Map();
    var_dump($builtin_map instanceof HH\Map); // True
  }

  Test\foo();
  bar();
}

