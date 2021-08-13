<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

abstract class MyBaseClass {
  public function test(): void {
    Code`(MyClass $x) ==> {
      $x->prop;
    }`;
  }
}

abstract class MyClass extends MyBaseClass {
  protected ExampleInt $prop;
}
