<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

<<__EntryPoint>>
function test(): void {
  require __DIR__.'/../../../hack/test/expr_tree.php';

  print_et(ExampleDsl`1 === 2`);
  print_et(ExampleDsl`1 !== 2`);
  print_et(ExampleDsl`1 + 2`);
  print_et(ExampleDsl`1 - 2`);
  print_et(ExampleDsl`1 * 2`);
  print_et(ExampleDsl`1 / 2`);
  print_et(ExampleDsl`1 % 2`);
  print_et(ExampleDsl`-1`);
  print_et(ExampleDsl`1 < 2`);
  print_et(ExampleDsl`1 <= 2`);
  print_et(ExampleDsl`1 > 2`);
  print_et(ExampleDsl`1 >= 2`);
  print_et(ExampleDsl`1 & 2`);
  print_et(ExampleDsl`1 | 2`);
  print_et(ExampleDsl`1 ^ 2`);
  print_et(ExampleDsl`1 << 2`);
  print_et(ExampleDsl`1 >> 2`);
  print_et(ExampleDsl`~1`);
  print_et(ExampleDsl`true && false`);
  print_et(ExampleDsl`true || false`);
  print_et(ExampleDsl`!true`);
  print_et(ExampleDsl`"Hello" . "World"`);
}
