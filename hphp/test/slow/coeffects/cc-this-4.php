<?hh

class A {
  function f()[this::T1::C1] {}
}

<<__EntryPoint>>
function main()[] {
  (new A)->f();
}
