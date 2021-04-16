<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(
  ExampleContext $_,
): ExprTree<Code, string, (function(ExampleString): ExampleInt)> {
  return
    Code`(ExampleString $x) ==> { return 1; }`;
}

class MyParent {
  public static function bar(ExampleContext $_):
    ExprTree<Code, string, (function(ExampleString): ExampleInt)>
  {
    return
      Code`(ExampleString $x) ==> { return 1; }`;
  }
}

class MyChild extends MyParent {}

<<__EntryPoint>>
function test(): void {
  require 'expression_tree.inc';

  $et = Code`foo("bar")`;
  print_et($et);

  $et = Code`MyParent::bar("hello")`;
  print_et($et);

  $et = Code`MyChild::bar("world")`;
  print_et($et);
}
