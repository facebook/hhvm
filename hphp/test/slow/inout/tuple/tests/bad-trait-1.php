<?hh

trait T {
  function foo(inout $x) {}
}

interface I {
  function foo(&$x);
}

class C implements I {
  use T;
}
