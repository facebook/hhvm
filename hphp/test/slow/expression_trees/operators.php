<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

<<__EntryPoint>>
function test(): void {
  require 'expression_tree.inc';

  print_et(Code`1 === 2`);
  print_et(Code`1 !== 2`);
  print_et(Code`1 + 2`);
  print_et(Code`1 - 2`);
  print_et(Code`1 * 2`);
  print_et(Code`1 / 2`);
  print_et(Code`1 % 2`);
  print_et(Code`-1`);
  print_et(Code`1 < 2`);
  print_et(Code`1 <= 2`);
  print_et(Code`1 > 2`);
  print_et(Code`1 >= 2`);
  print_et(Code`1 & 2`);
  print_et(Code`1 | 2`);
  print_et(Code`1 ^ 2`);
  print_et(Code`1 << 2`);
  print_et(Code`1 >> 2`);
  print_et(Code`~1`);
  print_et(Code`true && false`);
  print_et(Code`true || false`);
  print_et(Code`!true`);
  print_et(Code`"Hello" . "World"`);
}
