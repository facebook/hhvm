<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

final class MyClass {
  const FOO = "BAR";

  public static function helper(): ExampleExpression<ExampleString> {
    return ExampleDsl`static::FOO`;
  }
}

<<__EntryPoint>>
function test(): void {
  require __DIR__.'/../../../hack/test/expr_tree.php';

  $et = MyClass::helper();
  print_et($et);
}
