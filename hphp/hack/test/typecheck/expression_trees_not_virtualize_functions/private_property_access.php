<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

abstract class MyClass {
  private ExampleInt $prop;

  public function test(): void {
    ExampleDsl`(MyClass $x) ==> {
      $x->prop;
    }`;
  }
}
