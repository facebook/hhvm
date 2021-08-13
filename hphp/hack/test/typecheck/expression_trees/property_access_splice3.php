<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): ExprTree<Code, Code::TAst, ExampleInt> {
  throw new Exception();
}

abstract class MyClass {
  public function test(MyChildClass $x): void {
    Code`(): ExampleInt ==> {
      return ${ $x->prop };
    }`;
  }
}

abstract class MyChildClass extends MyClass {
  protected ExprTree<Code, Code::TAst, ExampleInt> $prop;

  public function __construct() {
    $this->prop = foo();
  }
}
