<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

  function test(): void {
    ExampleDsl`(MyClass $x) ==> {
      $x->fooo();
    }`;
  }

abstract class MyClass {
  public function foo(ExampleInt $x): void {}
}
