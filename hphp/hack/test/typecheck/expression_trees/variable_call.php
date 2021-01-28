<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): void {
  $fun_call = Code`((function(ExampleString): ExampleInt) $foo) ==> {
    $foo("baz");
  }`;
}
