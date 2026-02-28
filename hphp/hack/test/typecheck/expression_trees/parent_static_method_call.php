<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

class MyParent {
  public static async function bar(
    ExampleContext $_,
  ): Awaitable<ExampleExpression<ExampleFunction<(function(ExampleString): ExampleInt)>>> {
    throw new Exception();
  }
}

class MyClass extends MyParent {
  public function test(): void {
    $fun_call = ExampleDsl`parent::bar("baz")`;
  }
}
