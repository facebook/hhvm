<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(int $_): ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> {
  throw new Exception();
}

function bar(ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> $x): ExampleDsl<ExampleDsl, ExampleDsl::TAst, ExampleString> {
  throw new Exception();
}

function helper_baz(ExampleContext $_):
  Awaitable<ExprTree<ExampleDsl, ExampleDsl::TAst, (function(ExampleString): ExampleFloat)>>
{
  throw new Exception();
}

function test(): ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleFloat> {
  $et = 1
    |> ExampleDsl`helper_baz(${ foo($$) |> bar($$) })`;

  return $et;
}
