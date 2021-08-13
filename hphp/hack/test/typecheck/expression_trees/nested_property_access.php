<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  Code`(MyNestedState $x) ==> {
    $x->nest->my_prop = 1;
  }`;
}

abstract class MyNestedState {
  public MyState $nest;
}

abstract class MyState {
  public ExampleInt $my_prop;
}
