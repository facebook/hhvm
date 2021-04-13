<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

<<__EntryPoint>>
function test(): void {
  require 'expression_tree.inc';

  print_et(Code`5`);
  print_et(Code`"Hello"`);
  print_et(Code`3.14`);
  print_et(Code`false`);
  print_et(Code`null`);
  print_et(Code`() ==> { return; }`);
}
