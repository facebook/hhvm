<?hh

// Test that Pair behaves properly when mixing namespaced and
// non-namespaced code.

namespace Test {
  // Can have custom class named Pair.

  class Pair {
    public function __construct() {
      echo "Custom Pair\n";
    }
  }

  function foo() {
    $custom_set = new Pair();
    var_dump($custom_set instanceof \HH\Pair); // False
  }

}

namespace {

  function bar() {
    $builtin_set = Pair {1, 2};
    var_dump($builtin_set instanceof HH\Pair); // True
  }

  Test\foo();
  bar();
}

