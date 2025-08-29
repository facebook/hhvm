<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): ExampleExpression<ExampleInt> {
  throw new Exception();
}

abstract class MyClass {
  public function test(MyChildClass $x): void {
    ExampleDsl`(): ExampleInt ==> {
      return ${ $x->prop };
    }`;
  }
}

abstract class MyChildClass extends MyClass {
  protected ExampleExpression<ExampleInt> $prop;

  public function __construct() {
    $this->prop = foo();
  }
}
