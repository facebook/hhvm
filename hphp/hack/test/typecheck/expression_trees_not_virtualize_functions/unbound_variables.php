<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  $x = 1;

  // Expression Trees do not inherit local variables from the outer scope
  $_ = ExampleDsl`$x + 1`;
}

class MyClass {
  public function foo(): void {
    // We should not suggest $this here.
    ExampleDsl`$x`;
  }
}
