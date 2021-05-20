<?hh

function f<reify T>()[T::C] {}

<<__EntryPoint>>
function main()[] {
  f<int>();
}
