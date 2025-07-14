<?hh

<<file:__EnableUnstableFeatures('expression_trees', 'expression_tree_coalesce_operator')>>

<<__EntryPoint>>
function test(): void {
  require __DIR__.'/../../../hack/test/expr_tree.php';

  $et = ExampleDsl`(?ExampleInt $x, ExampleInt $y) ==> {
    null ?? null;
    null ?? 1;
    null ?? $y;
    $x ?? $y;
    $y ?? $x;
    // The coalesce operator is right-associative
    $x ?? null ?? $y;
    ($x ?? null) ?? $y;
    $x ?? (null ?? $y);
    // The addition operator has a higher precedence than the coalesce operator
    ($x ?? $y) + $y;
    // Fails because the result of the first expression is ?ExampleInt
    ($x ?? $x) + $y;
  }`;

  print_et($et);
}
