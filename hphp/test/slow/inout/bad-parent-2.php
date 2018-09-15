<?hh

class P {
  function foo($x) {}
}

class C extends P {
  function foo(inout $x) {}
}
