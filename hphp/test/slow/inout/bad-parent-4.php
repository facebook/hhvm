<?hh

class P {
  function foo(inout $x) {}
}

class C extends P {
  function foo(&$x) {}
}
