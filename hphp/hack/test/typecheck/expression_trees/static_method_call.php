<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

final class MyClass {
  public static async function bar(
    ExampleContext $_,
  ): Awaitable<ExprTree<Code, Code::TAst, (function(ExampleString): ExampleInt)>> {
    throw new Exception();
  }
}

function foo(): void {
  $fun_call = Code`MyClass::bar("baz")`;
}
