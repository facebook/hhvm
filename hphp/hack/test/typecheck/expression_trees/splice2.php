<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function lift<T>(T $_): ExprTree<Code, Code::TAst, T> {
  throw new Exception();
}

function test(): void {
  Code`${lift(1 << 4)}`;
}
