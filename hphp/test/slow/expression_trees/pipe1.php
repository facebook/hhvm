<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(int $i): ExprTree<Code, Code::TAst, ExampleInt> {
  print $i."\n";
  return Code`123`;
}

function bar(ExprTree<Code, Code::TAst, ExampleInt> $x): ExprTree<Code, Code::TAst, ExampleInt> {
  return Code`2 - ${ $x }`;
}

// Testing multiple $$ occurrences within many splices in ETs
function test(ExprTree<Code, Code::TAst, ExampleInt> $x): void {
  $et = $x |> Code`${ $$ } + ${ $$ |> bar($$) } + ${ 1 |> foo($$) }`;
  print_et($et);
}

<<__EntryPoint>>
function entrypoint(): void {
  require 'expression_tree.inc';
  test(Code`1`);
}
