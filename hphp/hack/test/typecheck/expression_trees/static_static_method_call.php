<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

abstract class MyClass {
  public abstract static function bar(
    ExampleContext $_,
  ): Awaitable<ExprTree<Code, Code::TAst, (function(ExampleString): ExampleInt)>>;

  public function test(): void {
    $fun_call = Code`static::bar("baz")`;
  }
}
