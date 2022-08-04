<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

final class MyClass {
  public static async function bar(
    ExampleContext $_,
  ): Awaitable<ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleFunction<(function(ExampleString): ExampleInt)>>> {
    throw new Exception();
  }
}

function foo(): void {
  $fun_call = ExampleDsl`MyClass::bar("baz")`;
}
