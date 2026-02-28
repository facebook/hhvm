<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(int $i): ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> {
  print $i."\n";
  return ExampleDsl`123`;
}

function bar(ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> $x): ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> {
  return ExampleDsl`2 - ${ $x }`;
}

// Testing multiple $$ occurrences within many splices in ETs
function test(ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> $x): void {
  $et = $x |?> ExampleDsl`${ $$ } + ${ $$ |?> bar($$) } + ${ 1 |?> foo($$) }`;
  print_et($et);
}

function test_null(?ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> $x): void {
  $et = $x |?> ExampleDsl`${ $$ } + ${ $$ |?> bar($$) } + ${ 1 |?> foo($$) }`;
  var_dump($et);
}

<<__EntryPoint>>
function entrypoint(): void {
  require __DIR__.'/../../../hack/test/expr_tree.php';
  test(ExampleDsl`1`);
  test_null(null);
}
