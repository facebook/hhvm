<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

abstract class MyClass {
  public abstract static function bar(
    ExampleContext $_,
  ): Awaitable<ExampleExpression<ExampleFunction<(function(ExampleString): ExampleInt)>>>;

  public function test(): void {
    $fun_call = ExampleDsl`static::bar("baz")`;
  }
}
