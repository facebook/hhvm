<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): void {
  $fun_call = ExampleDsl`((function(ExampleString): ExampleInt) $foo) ==> {
    $foo("baz");
  }`;
}
