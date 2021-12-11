<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

final class MyClass {
  public static async function bar(
    ExampleContext $_,
  ): Awaitable<ExprTree<ExampleDsl, ExampleDsl::TAst, (function(ExampleString): ExampleInt)>> {
    throw new Exception();
  }

  public function test(): void {
    $fun_call = ExampleDsl`self::bar("baz")`;
  }
}
