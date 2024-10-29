<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function f<T1, T2, T3>(string $s, ExprTree<T1, T2, T3> $et): ExprTree<T1, T2, T3> {
  print($s."\n");
  return $et;
}

<<__EntryPoint>>
function test(): void {
  require __DIR__.'/../../../hack/test/expr_tree.php';

  $et = f('a', ExampleDsl`1 + ${f('b', ExampleDsl`3 + ${f('c', ExampleDsl`4`)}`)}`);

  print_et($et);
}
