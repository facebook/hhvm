<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(int $_): ExprTree<Code, Code::TAst, ExampleInt> {
  throw new Exception();
}

function bar<T>(ExprTree<Code, Code::TAst, T> $x): ExprTree<Code, Code::TAst, T> {
  return $x;
}

function test(ExprTree<Code, Code::TAst, ExampleInt> $x): void {
  $x |> Code`${ $$ } + ${ $$ |> bar($$) } + ${ 1 |> foo($$) }`;
}
