<?hh

class A {
  const type T = int;
}

function f<reify T>()[T::T::C] :mixed{}

<<__EntryPoint>>
function main()[] :mixed{
  f<A>();
}
