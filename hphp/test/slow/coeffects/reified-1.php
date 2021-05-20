<?hh

class A1 {
  const ctx C = [rx];
  const type T1 = A1;
  const type T2 = A1;
}

class A2<reify T> {
  function f()[T::C] {}
}

function f<reify T>()[T::C] {}

function g<reify T>()[T::T1::T2::C] {}

<<__EntryPoint>>
function main()[] {
  f<A1>();
  (new A2<A1>())->f();
  g<A1>();
}
