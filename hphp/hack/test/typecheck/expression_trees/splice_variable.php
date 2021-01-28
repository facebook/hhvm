<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function lift<T>(T $_): ExprTree<Code, Code::TAst, T> {
  throw new Exception();
}

function test(): void {
  $x = 1;

  // Expression Trees do not inherit local variables from the outer scope
  // But splices do
  $_ = Code`${lift($x)}`;
}
