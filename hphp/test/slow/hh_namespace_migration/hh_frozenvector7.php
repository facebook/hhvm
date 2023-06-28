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

  function foo() :mixed{
    $custom_set = new ImmVector();
    \var_dump($custom_set is \HH\ImmVector); // False
  }

}

namespace {

  function bar() :mixed{
    $builtin_set = new ImmVector();
    \var_dump($builtin_set is HH\ImmVector); // True
  }

  <<__EntryPoint>> function main(): void {
  Test\foo();
  bar();
  }
}
