<?hh

// Test that FrozenVector behaves properly when mixing namespaced and
// non-namespaced code.

namespace Test {
  // Can have custom class named FrozenVector.

  class FrozenVector {
    public function __construct() {
      echo "Custom FrozenVector\n";
    }
  }

  function foo() {
    $custom_set = new FrozenVector();
    var_dump($custom_set instanceof \HH\FrozenVector); // False
  }

}

namespace {

  function bar() {
    $builtin_set = new FrozenVector();
    var_dump($builtin_set instanceof HH\FrozenVector); // True
  }

  Test\foo();
  bar();
}

