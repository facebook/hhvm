<?hh
<<file: __EnableUnstableFeatures('expression_trees')>>
<<file: __EnableUnstableFeatures('await_in_splice')>>

class C<T> {
  public function __construct(public T $x) {}
  public function get(): T { return $this->x; }
}

async function myTestFunction1(
  bool $b,
): Awaitable<C<ExampleExpression<ExampleInt>>> {
  return $b ? new C(ExampleDsl`${await myTestFunction2()}`) : new C(ExampleDsl`2`);
}

async function myTestFunction2(
): Awaitable<ExampleExpression<ExampleInt>> {
  echo "here\n";
  return ExampleDsl`1`;
}

<<__EntryPoint>>
async function myTestFunction(): Awaitable<void> {
  $x = ExampleDsl`${(await myTestFunction1(false))->get()}`;
}
