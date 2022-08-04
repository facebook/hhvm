<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  ExampleDsl`(MyState $x): ExampleInt ==> {
    return $x->my_prop + 1;
  }`;
}

abstract class MyState {
  public ExampleInt $my_prop;
}
