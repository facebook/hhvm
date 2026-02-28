<?hh

class A {
  function foo()[rx] :mixed{}
}

function bar(mixed $x)[ctx $x] :mixed{}

<<__EntryPoint>>
function main()[] :mixed{
  bar(meth_caller(A::class, 'foo'));
}
