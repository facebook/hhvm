<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

abstract class MyClass {
  protected ExampleInt $prop;

  public function test(): void {
    Code`(MyClass $x) ==> {
      $x->prop;
    }`;
  }
}
