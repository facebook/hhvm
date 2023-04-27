<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): void {
  $fun_call = ExampleDsl`(ExampleFunction<(function(ExampleString): ExampleInt)> $foo) ==> {
    $foo("baz");
  }`;
}
