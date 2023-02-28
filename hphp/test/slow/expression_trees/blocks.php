<?hh

<<file:__EnableUnstableFeatures('expression_trees', 'expression_tree_blocks')>>

<<__EntryPoint>>
function test(): void {
  require 'expression_tree.inc';

  $et = Code`{}`;
  print_et($et);

  $et = Code`{
    1 + 2;
    true || false;
    while(true) {};
  }`;
  print_et($et);
}
