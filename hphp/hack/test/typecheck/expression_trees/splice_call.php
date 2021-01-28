<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(
  ExprTree<Code, Code::TAst, (function(ExampleString): ExampleInt)> $x,
): void {
  $fun_call = Code`(${$x})("baz")`;
}
