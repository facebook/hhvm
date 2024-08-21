<?hh

<<file: __EnableUnstableFeatures('expression_trees')>>

<<__EntryPoint>>
function test(): void {
  require __DIR__.'/../../../hack/test/expr_tree.php';

  $et = ExampleDsl`(ExampleBool $z, ExampleBool $y = false) ==> {
    $x = $y;

    if ($x && $z) {
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
