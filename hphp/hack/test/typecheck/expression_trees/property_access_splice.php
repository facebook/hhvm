<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> {
  throw new Exception();
}

abstract class MyClass {
  protected ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> $prop;
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
