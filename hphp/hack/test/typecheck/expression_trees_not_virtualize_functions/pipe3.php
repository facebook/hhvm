<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(int $_): ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> {
  throw new Exception();
}

function bar(ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> $x): ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleString> {
  throw new Exception();
}

function test(): ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleFloat> {
  // Ensure that we still get some errors
  $et = 1
    |> ExampleDsl`${ foo($$) |> bar($$) |> bar($$) }`;

  return $et;
}
