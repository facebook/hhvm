<?hh
<<file:__EnableUnstableFeatures('expression_trees')>>
<<file:__EnableUnstableFeatures('await_in_splice')>>

function myTestFunction1(): ExprTree<Code, Code::TAst, ExampleInt> {
  return Code`2`;
}

async function myTestFunction2(): Awaitable<ExprTree<Code, Code::TAst, ExampleInt>> {
  return Code`1`;
}

<<__EntryPoint>>
async function myTestFunction(): Awaitable<void> {
  require 'expression_tree.inc';
  $x = Code`${await myTestFunction2()}`;
  $y = Code`1 + ${await myTestFunction2()} + ${myTestFunction1()}`;
  print_et($x);
  print_et($y);
}
