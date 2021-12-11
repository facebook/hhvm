<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

abstract class MyClass {
  private function foo(ExampleInt $x): void {}

  public function test(): void {
    ExampleDsl`(MyClass $x) ==> {
      $x->foo(1);
    }`;
  }
}
