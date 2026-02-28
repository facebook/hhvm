<?hh

<<file: __EnableUnstableFeatures('expression_trees')>>

function test(
  ExampleExpression<ExampleFunction<(function(int, string): void)>> $foo,
  ExampleExpression<ExampleFunction<(function(float, bool): int)>> $bar,
  ExampleExpression<ExampleFunction<(function(): float)>> $baz,
  ExampleExpression<ExampleFunction<(function(): bool)>> $qux,
  ExampleExpression<ExampleFunction<(function(): string)>> $qaal,
): void {
  ExampleDsl`${$foo}(${$bar}(${$baz}(), ${$qux}()), ${$qaal}())`;
}
