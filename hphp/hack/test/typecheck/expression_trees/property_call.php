<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  ExampleDsl`(MyState $x) ==> {
    return ($x->my_prop)(1);
  }`;
}

abstract class MyState {
  public ExampleFunction<(function(ExampleInt): ExampleInt)> $my_prop;
}
