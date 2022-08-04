<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

abstract class MyClass {
  public ExampleInt $prop;
}

function test(): void {
  ExampleDsl`(MyClass $x) ==> {
    $x->prop;
  }`;
}
