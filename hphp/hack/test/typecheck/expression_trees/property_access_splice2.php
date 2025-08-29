<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): ExampleExpression<ExampleInt> {
  throw new Exception();
}

abstract class MyClass {
  private ExampleExpression<ExampleInt> $prop;

  public function __construct() {
    $this->prop = foo();
  }

  public function test(MyClass $x): void {
    ExampleDsl`(): ExampleInt ==> {
      return ${ $this->prop };
    }`;
  }
}
