<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(int $_): ExprTree<Code, Code::TAst, ExampleInt> {
  throw new Exception();
}

function test(): void {
  1 |> Code`(): ExampleInt ==> { return ${ $$ |> foo($$) }; }`;
}
