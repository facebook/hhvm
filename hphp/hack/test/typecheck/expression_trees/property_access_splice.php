<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): ExampleExpression<ExampleInt> {
  throw new Exception();
}

abstract class MyClass {
  protected ExampleExpression<ExampleInt> $prop;
}

abstract class MyChildClass extends MyClass {
  public function __construct() {
    $this->prop = foo();
  }

  public function test(): void {
    ExampleDsl`(): ExampleInt ==> {
      return ${ $this->prop };
    }`;
  }
}
