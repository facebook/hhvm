<?hh

function bar(mixed $x)[ctx $x] {}

<<__EntryPoint>>
function main()[] {
  bar(meth_caller(A::class, 'foo'));
}
