<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(
  ExampleContext $_,
): Awaitable<ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleFunction<(function(int, string): void)>>> {
  throw new Exception();
}

function bar(
  ExampleContext $_,
): Awaitable<ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleFunction<(function(float, bool): int)>>> {
  throw new Exception();
}

function baz(
  ExampleContext $_,
): Awaitable<ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleFunction<(function(): float)>>> {
  throw new Exception();
}

function qux(
  ExampleContext $_,
): Awaitable<ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleFunction<(function(): bool)>>> {
  throw new Exception();
}

function qaal(
  ExampleContext $_,
): Awaitable<ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleFunction<(function(): string)>>> {
  throw new Exception();
}

function test(): void {
  ExampleDsl`foo(bar(baz(), qux()), qaal())`;
}
