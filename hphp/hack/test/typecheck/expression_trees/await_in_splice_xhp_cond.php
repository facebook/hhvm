<?hh
<<file:__EnableUnstableFeatures('expression_trees')>>
<<file: __EnableUnstableFeatures('await_in_splice')>>

class :xhp {
  public function __construct(
    dict<string, mixed> $attrs,
    vec<mixed> $children,
    string $file,
    int $line,
  ) {}
  attribute ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> x;
}

async function myTestFunction2(
): Awaitable<ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt>> {
  return ExampleDsl`1`;
}

async function f(): Awaitable<void> {
  true ? <xhp x = {ExampleDsl`${await myTestFunction2()}`}/> : false;
}
