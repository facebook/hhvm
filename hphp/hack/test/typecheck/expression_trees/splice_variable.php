<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function lift<T>(T $_): ExampleExpression<T> {
  throw new Exception();
}

function test(): void {
  $x = 1;

  // Expression Trees do not inherit local variables from the outer scope
  // But splices do
  $_ = ExampleDsl`${lift($x)}`;
}
