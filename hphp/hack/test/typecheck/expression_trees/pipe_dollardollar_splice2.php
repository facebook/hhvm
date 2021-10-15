<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(ExprTree<Code, Code::TAst, ExampleInt> $x): void {
  $x |> Code`${ $$ } + ${ $$ } + ${ $$ }`;
}
