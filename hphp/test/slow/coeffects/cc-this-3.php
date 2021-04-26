<?hh

class A {
  const ctx C2 = [rx];
  function f()[this::T1::T2::C1, this::C2] {}
}

<<__EntryPoint>>
function main()[] {
  (new A)->f();
}
