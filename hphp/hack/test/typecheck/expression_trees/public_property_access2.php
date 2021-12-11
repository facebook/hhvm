<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

abstract class MyBaseClass {
  public ExampleInt $prop;
}

abstract class MyClass extends MyBaseClass {}

function test(): void {
  ExampleDsl`(MyClass $x) ==> {
    $x->prop;
  }`;
}
