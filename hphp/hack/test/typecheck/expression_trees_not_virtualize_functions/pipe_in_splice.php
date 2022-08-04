<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(int $_): ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> {
  throw new Exception();
}

function test(): void {
  ExampleDsl`(): ExampleInt ==> { return ${ 1 |> foo($$) }; }`;
}
