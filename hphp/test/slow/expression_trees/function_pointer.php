<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(
  ExampleContext $_,
): ExprTree<ExampleDsl, string, (function(ExampleString): ExampleInt)> {
  return
    ExampleDsl`(ExampleString $x) ==> { return 1; }`;
}

class MyParent {
  public static function bar(ExampleContext $_):
    ExprTree<ExampleDsl, string, (function(ExampleString): ExampleInt)>
  {
    return
      ExampleDsl`(ExampleString $x) ==> { return 1; }`;
  }
}

class MyChild extends MyParent {}

<<__EntryPoint>>
function test(): void {
  require __DIR__.'/../../../hack/test/expr_tree.php';

  $et = ExampleDsl`foo("bar")`;
  print_et($et);

  $et = ExampleDsl`MyParent::bar("hello")`;
  print_et($et);

  $et = ExampleDsl`MyChild::bar("world")`;
  print_et($et);
}
