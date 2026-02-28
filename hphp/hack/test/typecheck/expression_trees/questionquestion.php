<?hh

<<file:
  __EnableUnstableFeatures(
    'expression_trees',
    'expression_tree_coalesce_operator',
  )>>

function test(): void {
  ExampleDsl`(?ExampleInt $x, ExampleInt $y) ==> {
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
}

function test_invalid(): void {
  // Array access expression is not allowed as the left operand of the coalesce operator
  $f = ExampleDsl`(dynamic $x) ==> {
    $x[0] ?? 1;
  }`;
}
