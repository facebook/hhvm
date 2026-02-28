<?hh
<<file: __EnableUnstableFeatures('expression_trees')>>
<<file: __EnableUnstableFeatures('await_in_splice')>>

async function myTestFunction1(
  bool $b,
): Awaitable<ExampleExpression<ExampleInt>> {
  return $b
    ? ExampleDsl`(
        $x ==> {
          return ${await myTestFunction2()};
        }
      )(2)`
    : ExampleDsl`2`;
}

async function myTestFunction2(
): Awaitable<ExampleExpression<ExampleInt>> {
  return ExampleDsl`1`;
}
