<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

<<__EntryPoint>>
function test(): void {
  require 'expression_tree.inc';

  $x = Code`1`;
  $et = Code`${ $x }`;

  print_et($et);
}
