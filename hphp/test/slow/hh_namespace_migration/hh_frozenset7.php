<?hh

// Test that ImmSet behaves properly when mixing namespaced and
// non-namespaced code.

namespace Test {
  // Can have custom class named ImmSet.

  class ImmSet {
    public function __construct() {
      echo "Custom ImmSet\n";
    }
  }

  function foo() {
    $custom_set = new ImmSet();
    var_dump($custom_set instanceof \HH\ImmSet); // False
  }

}

namespace {

  function bar() {
    $builtin_set = new ImmSet();
    var_dump($builtin_set instanceof HH\ImmSet); // True
  }

  Test\foo();
  bar();
}

