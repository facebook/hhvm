<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  1 |> ExampleDsl`() ==> { $z = 1; }`;
}
