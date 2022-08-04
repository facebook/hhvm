<?hh

<<file: __EnableUnstableFeatures('expression_trees')>>

function test(
  ExprTree<ExampleDsl, ExampleDsl::TAst, (function(int, string): void)> $foo,
  ExprTree<ExampleDsl, ExampleDsl::TAst, (function(float, bool): int)> $bar,
  ExprTree<ExampleDsl, ExampleDsl::TAst, (function(): float)> $baz,
  ExprTree<ExampleDsl, ExampleDsl::TAst, (function(): bool)> $qux,
  ExprTree<ExampleDsl, ExampleDsl::TAst, (function(): string)> $qaal,
): void {
  ExampleDsl`${$foo}(${$bar}(${$baz}(), ${$qux}()), ${$qaal}())`;
}
