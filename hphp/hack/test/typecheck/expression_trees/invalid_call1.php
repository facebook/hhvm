<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

final class MyClass {
  public static function bar(
    ExampleContext $_,
  ): ExprTree<Code, Code::TAst, (function(ExampleString): ExampleInt)> {
    throw new Exception();
  }
}

function foo(): void {
  $fun_call = Code`(MyClass $y) ==> {
    $y::bar("baz");
  }`;
}
