<?hh

interface I {
  function foo(inout $x);
}

class C implements I {
  function foo($x) {}
}
