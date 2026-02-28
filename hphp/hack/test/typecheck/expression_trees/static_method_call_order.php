<?hh

<<file: __EnableUnstableFeatures('expression_trees')>>

class MyExample {
  public static function foo(
    ExampleContext $_,
  ): Awaitable<ExampleExpression<ExampleFunction<(function(int, string): void)>>> {
    throw new Exception();
  }

  public static function bar(
    ExampleContext $_,
  ): Awaitable<ExampleExpression<ExampleFunction<(function(float, bool): int)>>> {
    throw new Exception();
  }

  public static function baz(
    ExampleContext $_,
  ): Awaitable<ExampleExpression<ExampleFunction<(function(): float)>>> {
    throw new Exception();
  }

  public static function qux(
    ExampleContext $_,
  ): Awaitable<ExampleExpression<ExampleFunction<(function(): bool)>>> {
    throw new Exception();
  }

  public static function qaal(
    ExampleContext $_,
  ): Awaitable<ExampleExpression<ExampleFunction<(function(): string)>>> {
    throw new Exception();
  }
}

function test(): void {
  ExampleDsl`MyExample::foo(
    MyExample::bar(MyExample::baz(), MyExample::qux()),
    MyExample::qaal(),
  )`;
}
