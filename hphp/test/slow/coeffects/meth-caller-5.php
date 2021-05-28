<?hh

class A {
  const ctx C = [];
  function foo()[this::C] {}
}

function bar(mixed $x)[ctx $x] {}

<<__EntryPoint>>
function main()[] {
  bar(meth_caller(A::class, 'foo'));
}
