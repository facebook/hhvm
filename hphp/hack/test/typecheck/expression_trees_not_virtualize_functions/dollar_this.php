<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

class MyClass {
  public function foo(): void {
    ExampleDsl`$this`;
  }
}
