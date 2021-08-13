<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

abstract class MyBaseClass {
  protected ExampleInt $prop;
}

abstract class MyClass extends MyBaseClass {
  public function test(): void {
    Code`(MyClass $x) ==> {
      $x->prop;
    }`;
  }
}
