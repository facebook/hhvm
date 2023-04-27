<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

final class MyClass {
  public static function bar(
    ExampleContext $_,
  ): ExprTree<ExampleDsl, ExampleDsl::TAst, (function(ExampleString): ExampleInt)> {
    throw new Exception();
  }
}

function foo(): void {
  $fun_call = ExampleDsl`((function(): MyClass) $y) ==> {
    ($y())::bar("baz");
  }`;
}
