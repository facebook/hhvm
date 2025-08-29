<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(ExampleExpression<string> $y): void {
  ExampleDsl`(MyState $x) ==> {
    $z = ${ $y };
    return $x->$z;
  }`;
}

abstract class MyState {
  public function foo(ExampleInt $x): void {}
}
