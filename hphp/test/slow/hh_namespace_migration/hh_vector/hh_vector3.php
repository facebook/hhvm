<?hh

namespace A {
  class Vector {
    function foo() {
      echo "foo\n";
    }
  }

  $x = new Vector();
  $x->foo();
}
