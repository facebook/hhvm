<?hh
<<file: __EnableUnstableFeatures('expression_trees')>>
<<file: __EnableUnstableFeatures('await_in_splice')>>

async function myTestFunction2(
): Awaitable<ExprTree<Code, Code::TAst, ExampleInt>> {
  echo 1;
  await RescheduleWaitHandle::create(0, 0);
  echo 2;
  return Code`2`;
}

<<__EntryPoint>>
async function myTestFunction(): Awaitable<void> {
  require 'expression_tree.inc';
  $y = Code`
    ${await myTestFunction2()} +
    ${await myTestFunction2()}
  `;
}
