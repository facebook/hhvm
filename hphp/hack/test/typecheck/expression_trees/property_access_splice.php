<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): ExprTree<Code, Code::TAst, ExampleInt> {
  throw new Exception();
}

abstract class MyClass {
  protected ExprTree<Code, Code::TAst, ExampleInt> $prop;
}

abstract class MyChildClass extends MyClass {
  public function __construct() {
    $this->prop = foo();
  }

  public function test(): void {
    Code`(): ExampleInt ==> {
      return ${ $this->prop };
    }`;
  }
}
