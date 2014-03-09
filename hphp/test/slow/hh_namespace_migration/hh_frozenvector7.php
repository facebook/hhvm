<?hh

// Test that ImmVector behaves properly when mixing namespaced and
// non-namespaced code.

namespace Test {
  // Can have custom class named ImmVector.

  class ImmVector {
    public function __construct() {
      echo "Custom ImmVector\n";
    }
  }

  function foo() {
    $custom_set = new ImmVector();
    var_dump($custom_set instanceof \HH\ImmVector); // False
  }

}

namespace {

  function bar() {
    $builtin_set = new ImmVector();
    var_dump($builtin_set instanceof HH\ImmVector); // True
  }

  Test\foo();
  bar();
}

