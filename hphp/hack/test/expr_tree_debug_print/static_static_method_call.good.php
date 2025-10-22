<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

final class MyClass {
  public static function bar(
  ): ExampleExpression<ExampleFunction<(function(ExampleString): ExampleInt)>> {
    throw new Exception();
  }

  public function test(): void {
    $fun_call = ExampleDsl`static::bar("baz")`;
  }
}
