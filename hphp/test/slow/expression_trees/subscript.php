<?hh

<<file:__EnableUnstableFeatures('expression_trees', 'expression_tree_subscript')>>

<<__EntryPoint>>
function test(): void {
  require __DIR__.'/../../../hack/test/expr_tree.php';

  // Basic vec subscript
  print_et(ExampleDsl`(vec<ExampleInt> $arr, ExampleInt $idx) ==> $arr[$idx]`);

  // Basic dict subscript (string key)
  print_et(ExampleDsl`(dict<ExampleString, ExampleInt> $map, ExampleString $key) ==> $map[$key]`);

  // Dict subscript with int key
  print_et(ExampleDsl`(dict<ExampleInt, ExampleString> $map, ExampleInt $key) ==> $map[$key]`);

  // Nested subscript
  print_et(ExampleDsl`(vec<vec<ExampleInt>> $arr, ExampleInt $i, ExampleInt $j) ==> $arr[$i][$j]`);

  // Subscript in arithmetic
  print_et(ExampleDsl`(vec<ExampleInt> $arr, ExampleInt $idx) ==> $arr[$idx] + 1`);

  // Subscript on property access
  print_et(ExampleDsl`(MyContainer $c, ExampleInt $idx) ==> $c->items[$idx]`);
}

abstract class MyContainer {
  public vec<ExampleInt> $items;
}
