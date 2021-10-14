<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  1 |> Code`() ==> { $z = 1; }`;
}
