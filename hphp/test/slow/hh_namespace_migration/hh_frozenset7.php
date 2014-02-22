<?hh

// Test that FixedSet behaves properly when mixing namespaced and
// non-namespaced code.

namespace Test {
  // Can have custom class named FixedSet.

  class FixedSet {
    public function __construct() {
      echo "Custom FixedSet\n";
    }
  }

  function foo() {
    $custom_set = new FixedSet();
    var_dump($custom_set instanceof \HH\FixedSet); // False
  }

}

namespace {

  function bar() {
    $builtin_set = new FixedSet();
    var_dump($builtin_set instanceof HH\FixedSet); // True
  }

  Test\foo();
  bar();
}

