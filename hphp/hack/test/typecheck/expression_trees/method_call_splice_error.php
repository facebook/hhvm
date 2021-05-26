<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

class Foo {
  public function boo(): void {}
}

function test(): void {
  $x = new Foo();
  Code`${ $x->bar() }`;
}
