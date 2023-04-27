<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(int $_): ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> {
  throw new Exception();
}

function bar(ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> $x): ExampleDsl<ExampleDsl, ExampleDsl::TAst, ExampleString> {
  throw new Exception();
}

function test(): ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleString> {
  $et = 1
    |> $$ . "Hello" . $$
    |> (int)$$
    |> ExampleDsl`${ foo($$) |> bar($$) }`;

  return $et;
}
