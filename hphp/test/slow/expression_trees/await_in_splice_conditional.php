<?hh
<<file: __EnableUnstableFeatures('expression_trees')>>
<<file: __EnableUnstableFeatures('await_in_splice')>>

class C<T> {
  public function __construct(public T $x) {}
}

async function myTestFunction1(
  bool $b,
): Awaitable<ExprTree<Code, Code::TAst, ExampleInt>> {
  return $b ? new C(Code`${await myTestFunction2()}`) : Code`2`;
}

async function myTestFunction2(
): Awaitable<ExprTree<Code, Code::TAst, ExampleInt>> {
  echo "here\n";
  return Code`1`;
}

<<__EntryPoint>>
async function myTestFunction(): Awaitable<void> {
  require 'expression_tree.inc';
  $x = Code`${await myTestFunction1(false)}`;
  print_et($x);
}
