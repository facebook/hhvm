<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function bar(ExprTree<Code, Code::TAst, ExampleInt> $x): ExprTree<Code, Code::TAst, ExampleInt> {
  return Code`2 - ${ $x }`;
}

function combine(ExprTree<Code, Code::TAst, ExampleInt> $x, ExprTree<Code, Code::TAst, ExampleInt> $y): ExprTree<Code, Code::TAst, ExampleInt> {
  return Code`${ $x } * ${ $y }`;
}

// Testing multiple pipes within the splices
function test(ExprTree<Code, Code::TAst, ExampleInt> $x): void {
  // (2 - (2 - (1 * (2 - 1))) * (2 - (1 * (2 - 1))))
  $et = $x |> Code`${ combine($$, bar($$)) |> combine(bar($$), bar($$)) |> bar($$) }`;
  print_et($et);
}

<<__EntryPoint>>
function entrypoint(): void {
  require 'expression_tree.inc';
  test(Code`1`);
}
