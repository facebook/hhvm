<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

// Testing to make sure lambdas bodies don't capture $$ but call arguments do
function test(ExprTree<Code, Code::TAst, ExampleInt> $x): void {
  // 3
  $et = $x |> Code`${ (($x) ==> { return $x; })($$) }`;

  print_et($et);
}

<<__EntryPoint>>
function entrypoint(): void {
  require 'expression_tree.inc';
  test(Code`3`);
}
