<?hh

// Test that Set behaves properly when mixing namespaced and
// non-namespaced code.

namespace Test {
  // Can have custom class named Set.

  class Set {
    public function __construct() {
      echo "Custom Set\n";
    }
  }

  function foo() {
    $custom_set = new Set();
    var_dump($custom_set instanceof \HH\Set); // False
  }

}

namespace {

  function bar() {
    $builtin_set = new Set();
    var_dump($builtin_set instanceof HH\Set); // True
  }

  Test\foo();
  bar();
}

