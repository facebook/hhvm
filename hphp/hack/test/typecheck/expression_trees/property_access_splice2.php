<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): ExprTree<Code, Code::TAst, ExampleInt> {
  throw new Exception();
}

abstract class MyClass {
  private ExprTree<Code, Code::TAst, ExampleInt> $prop;

  public function __construct() {
    $this->prop = foo();
  }

  public function test(MyClass $x): void {
    Code`(): ExampleInt ==> {
      return ${ $this->prop };
    }`;
  }
}
