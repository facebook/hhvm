<?hh

<<file: __EnableUnstableFeatures('expression_trees')>>

class MyExample {
  public static function foo(
    ExampleContext $_,
  ): Awaitable<ExprTree<Code, Code::TAst, (function(int, string): void)>> {
    throw new Exception();
  }

  public static function bar(
    ExampleContext $_,
  ): Awaitable<ExprTree<Code, Code::TAst, (function(float, bool): int)>> {
    throw new Exception();
  }

  public static function baz(
    ExampleContext $_,
  ): Awaitable<ExprTree<Code, Code::TAst, (function(): float)>> {
    throw new Exception();
  }

  public static function qux(
    ExampleContext $_,
  ): Awaitable<ExprTree<Code, Code::TAst, (function(): bool)>> {
    throw new Exception();
  }

  public static function qaal(
    ExampleContext $_,
  ): Awaitable<ExprTree<Code, Code::TAst, (function(): string)>> {
    throw new Exception();
  }
}

function test(): void {
  Code`MyExample::foo(
    MyExample::bar(MyExample::baz(), MyExample::qux()),
    MyExample::qaal(),
  )`;
}
