<?hh

// Test that FrozenSet behaves properly when mixing namespaced and
// non-namespaced code.

namespace Test {
  // Can have custom class named FrozenSet.

  class FrozenSet {
    public function __construct() {
      echo "Custom FrozenSet\n";
    }
  }

  function foo() {
    $custom_set = new FrozenSet();
    var_dump($custom_set instanceof \HH\FrozenSet); // False
  }

}

namespace {

  function bar() {
    $builtin_set = new FrozenSet();
    var_dump($builtin_set instanceof HH\FrozenSet); // True
  }

  Test\foo();
  bar();
}

