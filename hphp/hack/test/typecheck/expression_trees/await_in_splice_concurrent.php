<?hh
<<file: __EnableUnstableFeatures('expression_trees')>>
<<file: __EnableUnstableFeatures('await_in_splice')>>

async function myTestFunction2(
): Awaitable<ExampleExpression<ExampleInt>> {
  echo 1;
  await RescheduleWaitHandle::create(0, 0);
  echo 2;
  return ExampleDsl`2`;
}

<<__EntryPoint>>
async function myTestFunction(): Awaitable<void> {
  $y = ExampleDsl`${await myTestFunction2()} + ${await myTestFunction2()}`;
}
