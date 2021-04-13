<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

<<__EntryPoint>>
function test(): void {
  require 'expression_tree.inc';

  $et = Code`(ExampleBool $y) ==> {
    $x = $y;

    if ($x) {
      return;
    }

    for ($i = 0; $i < 2; $i = $i + 1) {
      break;
    }

    while($x) {
      continue;
    }
  }`;

  print_et($et);

}
