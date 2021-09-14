<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(ExprTree<Code, Code::TAst, string> $y): void {
  Code`(MyState $x) ==> {
    $z = ${ $y };
    return $x->$z;
  }`;
}

abstract class MyState {
  public function foo(ExampleInt $x): void {}
}
