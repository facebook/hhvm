<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> $x): void {
  $x |> ExampleDsl`${ $$ }`;
}
