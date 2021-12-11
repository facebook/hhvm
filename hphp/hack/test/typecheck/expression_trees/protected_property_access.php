<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

abstract class MyBaseClass {
  protected ExampleInt $prop;
}

abstract class MyClass extends MyBaseClass {
  public function test(): void {
    ExampleDsl`(MyClass $x) ==> {
      $x->prop;
    }`;
  }
}
