<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function bar(string $_): ExprTree<Code, Code::TAst, ExampleInt> {
  throw new Exception();
}

function foo(): void {
  $z = Code`4 + ${bar('hello' + 1)}`;
}
