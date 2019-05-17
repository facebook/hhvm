<?hh

class P {
  function foo() {}
}

class C extends P {
  function foo(inout $x) {}
}

<<__EntryPoint>> function main(): void {}
