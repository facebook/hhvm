<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

abstract class MyClass {
  public ExampleInt $prop;
}

function test(): void {
  Code`(MyClass $x) ==> {
    $x->prop;
  }`;
}
