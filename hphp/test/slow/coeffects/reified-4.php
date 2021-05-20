<?hh

class A {
  const type T = int;
}

function f<reify T>()[T::T::C] {}

<<__EntryPoint>>
function main()[] {
  f<A>();
}
