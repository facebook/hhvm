<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(int $_): ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> {
  throw new Exception();
}

function bar<T>(ExprTree<ExampleDsl, ExampleDsl::TAst, T> $x): ExprTree<ExampleDsl, ExampleDsl::TAst, T> {
  return $x;
}

function test(ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> $x): void {
  $x |> ExampleDsl`${ $$ } + ${ $$ |> bar($$) } + ${ 1 |> foo($$) }`;
}
