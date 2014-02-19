<?hh

// Test that FixedVector behaves properly when mixing namespaced and
// non-namespaced code.

namespace Test {
  // Can have custom class named FixedVector.

  class FixedVector {
    public function __construct() {
      echo "Custom FixedVector\n";
    }
  }

  function foo() {
    $custom_set = new FixedVector();
    var_dump($custom_set instanceof \HH\FixedVector); // False
  }

}

namespace {

  function bar() {
    $builtin_set = new FixedVector();
    var_dump($builtin_set instanceof HH\FixedVector); // True
  }

  Test\foo();
  bar();
}

