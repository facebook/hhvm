<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(ExprTree<Code, Code::TAst, ExampleInt> $x): ExprTree<Code, Code::TAst, ExampleInt> {
  return Code`${ $x } + 7`;
}

function bar(ExprTree<Code, Code::TAst, ExampleInt> $x): ExprTree<Code, Code::TAst, ExampleInt> {
  return Code`2 - ${ $x }`;
}

function combine(ExprTree<Code, Code::TAst, ExampleInt> $x, ExprTree<Code, Code::TAst, ExampleInt> $y): ExprTree<Code, Code::TAst, ExampleInt> {
  return Code`${ $x } * ${ $y }`;
}

// Testing multiple pipes in the function
function test(ExprTree<Code, Code::TAst, ExampleInt> $x): void {
  // ((2 - 3) * ((100 / 3) * 3))
  $et = $x |> combine(bar($$), combine(Code`100 / ${ $$ }`, $$));
  print_et($et);

  // ((2 - 200) * ((100 / 200) * 200))
  $et = Code`200` |> combine(bar($$), combine(Code`100 / ${ $$ }`, $$));
  print_et($et);

  // ((2 - 300) * ((100 / 300) * 300))
  $et = Code`300` |> combine(bar($$), combine(Code`100 / ${ $$ }`, $$));
  print_et($et);
}

<<__EntryPoint>>
function entrypoint(): void {
  require 'expression_tree.inc';
  test(Code`3`);
}
