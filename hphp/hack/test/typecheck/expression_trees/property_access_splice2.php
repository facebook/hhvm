<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> {
  throw new Exception();
}

abstract class MyClass {
  private ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> $prop;

  public function __construct() {
    $this->prop = foo();
  }

  public function test(MyClass $x): void {
    ExampleDsl`(): ExampleInt ==> {
      return ${ $this->prop };
    }`;
  }
}
