<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

class Foo {
  public static async function bar(ExampleContext $_):
    Awaitable<ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleFunction<(function(): void)>>>
  {
    throw new Exception();
  }
}

function baz(ExampleContext $_):
  Awaitable<ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleFunction<(function(): void)>>>
{
  throw new Exception();
}

function test<T>(
  Spliceable<ExampleDsl, ExampleDsl::TAst, ExampleFunction<(function(T): T)>> $x,
): void {
  ExampleDsl`${ $x }(Foo::bar())`;
  ExampleDsl`${ $x }(baz())`;
}
