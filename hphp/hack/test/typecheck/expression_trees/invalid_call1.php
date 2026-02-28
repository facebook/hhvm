<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

final class MyClass {
  public static function bar(
    ExampleContext $_,
  ): ExampleExpression<(function(ExampleString): ExampleInt)> {
    throw new Exception();
  }
}

function foo(): void {
  $fun_call = ExampleDsl`(MyClass $y) ==> {
    $y::bar("baz");
  }`;
}
