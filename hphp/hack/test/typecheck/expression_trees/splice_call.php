<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(
  ExprTree<ExampleDsl, ExampleDsl::TAst, (function(ExampleString): ExampleInt)> $x,
): void {
  $fun_call = ExampleDsl`(${$x})("baz")`;
}
