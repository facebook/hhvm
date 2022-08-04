<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(int $_): ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> {
  throw new Exception();
}

function test(): void {
  1 |> ExampleDsl`(): ExampleInt ==> { return ${ $$ |> foo($$) }; }`;
}
