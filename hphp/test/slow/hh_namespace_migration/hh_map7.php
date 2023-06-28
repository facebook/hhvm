<?hh

// Test that Map behaves properly when mixing namespaced and
// non-namespaced code.

namespace Test {
  // Can have custom class named Map.

  class Map {
    public function __construct() {
      echo "Custom Map\n";
    }
  }

  function foo() :mixed{
    $custom_map = new Map();
    \var_dump($custom_map is \HH\Map); // False
  }

}

namespace {

  function bar() :mixed{
    $builtin_map = new Map();
    \var_dump($builtin_map is HH\Map); // True
  }

  <<__EntryPoint>> function main(): void {
  Test\foo();
  bar();
  }
}
