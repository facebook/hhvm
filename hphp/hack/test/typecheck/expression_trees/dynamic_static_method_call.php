<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

final class MyClass {
  public static async function bar(ExampleContext $_): Awaitable<
    ExampleExpression<ExampleFunction<(function(ExampleString): ExampleInt)>>,
  > {
    throw new Exception();
  }

  public function test(): void {
    $fun_call = ExampleDsl`{
      $cls = MyClass::class;
      $cls::bar("baz");
    }`;
  }
}
