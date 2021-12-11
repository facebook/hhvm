<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function bar(string $_): ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> {
  throw new Exception();
}

function foo(): void {
  $z = ExampleDsl`4 + ${bar('hello' + 1)}`;
}
