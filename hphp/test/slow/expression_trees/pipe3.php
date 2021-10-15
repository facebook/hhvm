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

// Testing multiple layers of pipes
function test(ExprTree<Code, Code::TAst, ExampleInt> $x): void {
  /*
    ((2 - (100 + (2 - ((3 + 7) * (3 + 7))) + 100))
      * (2 - (100 + (2 - ((3 + 7) * (3 + 7))) + 100)))
  */
  $et = $x
          |> foo($$)
          |> Code`100 + ${ combine($$, $$) |> bar($$) } + 100`
          |> bar($$)
          |> combine($$, $$);
  print_et($et);
}

<<__EntryPoint>>
function entrypoint(): void {
  require 'expression_tree.inc';
  test(Code`3`);
}
