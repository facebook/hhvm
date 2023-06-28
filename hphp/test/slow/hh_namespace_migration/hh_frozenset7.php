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

  function foo() :mixed{
    $custom_set = new ImmSet();
    \var_dump($custom_set is \HH\ImmSet); // False
  }

}

namespace {

  function bar() :mixed{
    $builtin_set = new ImmSet();
    \var_dump($builtin_set is HH\ImmSet); // True
  }

  <<__EntryPoint>> function main(): void {
  Test\foo();
  bar();
  }
}
