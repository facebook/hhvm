<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function lift<T>(T $_): ExampleExpression<T> {
  throw new Exception();
}

function test(): void {
  ExampleDsl`${lift(1 << 4)}`;
}
