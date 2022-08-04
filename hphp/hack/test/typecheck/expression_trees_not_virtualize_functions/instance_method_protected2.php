<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

abstract class MyParent {
  public function test(): void {
    ExampleDsl`(MyClass $x) ==> {
      $x->foo();
    }`;
  }
}

abstract class MyClass extends MyParent {
  protected function foo(ExampleInt $x): void {}
}
