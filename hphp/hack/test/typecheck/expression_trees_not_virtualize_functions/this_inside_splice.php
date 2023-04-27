<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function return_nothing(mixed $_): nothing {
  throw new Exception();
}

class MyClass {
  public function foo(): void {
    // We should be able to use $this inside splices.
    ExampleDsl`${return_nothing($this)}`;
  }
}
