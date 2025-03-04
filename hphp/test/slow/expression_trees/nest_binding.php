<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>
<<file:__EnableUnstableFeatures('expression_tree_nested_bindings')>>

async function f<T1, T2, T3>(ExprTree<T1, T2, T3> $et): Awaitable<ExprTree<T1, T2, T3>> {
  return $et;
}

<<__EntryPoint>>
async function test(): Awaitable<void> {
  require __DIR__.'/../../../hack/test/expr_tree.php';

  $et1 = ExampleDsl`{$y = 1; return ${ExampleDsl`1 + $y`};}`;
  print_et($et1);

  $et2 = ExampleDsl`{$y = 1; return ${ExampleDsl`1 + ${ExampleDsl`2 + $y`}`};}`;
  print_et($et2);

  $et3 = ExampleDsl`{$y = 1; return ${await f(ExampleDsl`1 + $y`)};}`;
  print_et($et3);

  $et4 = ExampleDsl`{$y = 1; return ${ExampleDsl`1 + ${await f(ExampleDsl`2 + $y`)}`};}`;
  print_et($et4);
}
