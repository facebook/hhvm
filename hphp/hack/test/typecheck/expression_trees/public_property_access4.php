<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

abstract class MyBaseClass {
  private int $prop = 1;
}

abstract class MyClass extends MyBaseClass {
  public ExampleInt $prop;

  public function test(): void {
    Code`(MyClass $x) ==> {
      $x->prop;
    }`;
  }
}
