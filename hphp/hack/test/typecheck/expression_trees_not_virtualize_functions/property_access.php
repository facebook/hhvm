<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  ExampleDsl`(MyState $x) ==> {
    $x->my_prop = 1;
  }`;
}

abstract class MyState {
  public ExampleInt $my_prop;
}
