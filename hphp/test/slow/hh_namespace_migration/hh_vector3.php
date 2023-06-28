<?hh

namespace A {
  class Vector {
    function foo() :mixed{
      echo "foo\n";
    }
  }

  <<__EntryPoint>> function main(): void {
  $x = new Vector();
  $x->foo();
  }
}
