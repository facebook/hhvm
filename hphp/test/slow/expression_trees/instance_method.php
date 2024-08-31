<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(
  ExampleContext $_,
): ExprTree<ExampleDsl, string, (function(ExampleString): ExampleInt)> {
  return
    ExampleDsl`(ExampleString $x) ==> { return 1; }`;
}

class MyParent {
  public function bar(ExampleContext $_):
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
  $p = new MyParent();
  $c = new MyChild();
  $et = ExampleDsl`$p->bar("hello")`;
  print_et($et);

  $et = ExampleDsl`$c->bar("world")`;
  print_et($et);
}
