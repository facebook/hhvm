<?hh

class B {
  const type T2 = B;
  const ctx C1 = [rx];
}

class A {
  const type T1 = B;
  const ctx C2 = [];
  function f()[this::T1::T2::C1, this::C2] :mixed{}
}

<<__EntryPoint>>
function main()[] :mixed{
  (new A)->f();
}
