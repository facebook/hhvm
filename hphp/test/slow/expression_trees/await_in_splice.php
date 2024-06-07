<?hh
<<file:__EnableUnstableFeatures('expression_trees')>>
<<file:__EnableUnstableFeatures('await_in_splice')>>

async function myTestFunction2(): Awaitable<ExprTree<Code, Code::TAst, ExampleInt>> {
  return Code`1`;
}

<<__EntryPoint>>
async function myTestFunction(): Awaitable<void> {
  require 'expression_tree.inc';
  $y = Code`1 + ${await myTestFunction2()}`;
  print_et($y);
}
