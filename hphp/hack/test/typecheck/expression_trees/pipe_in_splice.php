<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(int $_): ExprTree<Code, Code::TAst, ExampleInt> {
  throw new Exception();
}

function test(): void {
  Code`(): ExampleInt ==> { return ${ 1 |> foo($$) }; }`;
}
