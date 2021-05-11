<?hh

class A {
  const type T1 = int;
  function f()[this::T1::C1] {}
}

<<__EntryPoint>>
function main()[] {
  (new A)->f();
}
