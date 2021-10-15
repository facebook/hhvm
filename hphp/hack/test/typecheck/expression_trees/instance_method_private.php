<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  Code`(MyClass $x) ==> {
    $x->foo(1);
  }`;
}

abstract class MyClass {
  private function foo(ExampleInt $x): void {}
}
