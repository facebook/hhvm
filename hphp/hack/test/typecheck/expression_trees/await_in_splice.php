<?hh
<<file: __EnableUnstableFeatures('expression_trees')>>
<<file: __EnableUnstableFeatures('await_in_splice')>>

function myTestFunction1(): ExampleExpression<ExampleInt> {
  return ExampleDsl`2`;
}

async function myTestFunction2(
): Awaitable<ExampleExpression<ExampleInt>> {
  return ExampleDsl`1`;
}

class :xhp {
  public function __construct(
    dict<string, mixed> $attrs,
    vec<mixed> $children,
    string $file,
    int $line,
  ) {}

  attribute ExampleExpression<ExampleInt> x;
}

<<__EntryPoint>>
async function myTestFunction(): Awaitable<void> {
  ExampleDsl`${await myTestFunction2()}`;
  ExampleDsl`1 + ${await myTestFunction2()} + ${myTestFunction1()}`;
  <xhp x = {ExampleDsl`${await myTestFunction2()}`}/>;
}
