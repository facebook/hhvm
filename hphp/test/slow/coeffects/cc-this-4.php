<?hh

class A {
  function f()[this::T1::C1] :mixed{}
}

<<__EntryPoint>>
function main()[] :mixed{
  (new A)->f();
}
