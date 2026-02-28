<?hh

<<file:__EnableUnstableFeatures(
  'expression_trees',
  'capture_pipe_variables',
  'allow_extended_await_syntax',
  'allow_conditional_await_syntax',
)>>

class C<T> {
  public function __construct(public T $x) {}
}

async function myTestFunction1(
  bool $b,
): Awaitable<ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt>> {
  return $b ? new C(ExampleDsl`${await myTestFunction2()}`) : ExampleDsl`2`;
}

async function myTestFunction2(
): Awaitable<ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt>> {
  echo "here\n";
  return ExampleDsl`1`;
}

<<__EntryPoint>>
async function myTestFunction(): Awaitable<void> {
  require __DIR__.'/../../../hack/test/expr_tree.php';
  $x = ExampleDsl`${await myTestFunction1(false)}`;
  print_et($x);
}
