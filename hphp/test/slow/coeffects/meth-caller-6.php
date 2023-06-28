<?hh

class A {
  const ctx C = [];
  function foo()[this::C] :mixed{}
}

function bar(mixed $x)[ctx $x] :mixed{}

<<__EntryPoint>>
function main()[] :mixed{
  bar(meth_caller(A::class, 'foo'));
}
