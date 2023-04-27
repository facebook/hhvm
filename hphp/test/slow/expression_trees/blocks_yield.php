<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

<<__EntryPoint>>
function test(): void {
  require 'expression_tree.inc';

  $et = Code`{
    1 + 2;
    true || false;
    yield 2;
    while(true) {};
  }`;
  print_et($et);
}
