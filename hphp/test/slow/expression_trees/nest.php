<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>
<<file:__EnableUnstableFeatures('expression_tree_nest')>>

function f<T1, T2, T3>(string $s, ExprTree<T1, T2, T3> $et): ExprTree<T1, T2, T3> {
  print($s."\n");
  return $et;
}

<<__EntryPoint>>
function test(): void {
  require 'expression_tree.inc';

  $et = f('a', Code`1 + ${f('b', `3 + ${f('c', `4`)}`)}`);

  print_et($et);
}
