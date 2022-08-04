<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function lift<T>(T $_): ExprTree<ExampleDsl, ExampleDsl::TAst, T> {
  throw new Exception();
}

function test(): void {
  ExampleDsl`${lift(1 << 4)}`;
}
