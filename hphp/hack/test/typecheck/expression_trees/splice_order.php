<?hh

<<file: __EnableUnstableFeatures('expression_trees')>>

function test(
  ExprTree<Code, Code::TAst, (function(int, string): void)> $foo,
  ExprTree<Code, Code::TAst, (function(float, bool): int)> $bar,
  ExprTree<Code, Code::TAst, (function(): float)> $baz,
  ExprTree<Code, Code::TAst, (function(): bool)> $qux,
  ExprTree<Code, Code::TAst, (function(): string)> $qaal,
): void {
  Code`${$foo}(${$bar}(${$baz}(), ${$qux}()), ${$qaal}())`;
}
