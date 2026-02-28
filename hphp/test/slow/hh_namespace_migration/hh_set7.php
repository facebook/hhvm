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

  function foo() :mixed{
    $custom_set = new Set();
    \var_dump($custom_set is \HH\Set); // False
  }

}

namespace {

  function bar() :mixed{
    $builtin_set = new Set();
    \var_dump($builtin_set is HH\Set); // True
  }

  <<__EntryPoint>> function main(): void {
  Test\foo();
  bar();
  }
}
