<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): void {
  $x = () ==> ExampleDsl`4`;

  $z = ExampleDsl`4 + ${$x()}`;
}
