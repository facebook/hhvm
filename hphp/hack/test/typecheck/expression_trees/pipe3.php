<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(int $_): ExampleExpression<ExampleInt> {
  throw new Exception();
}

function bar(ExampleExpression<ExampleInt> $x): ExampleExpression<ExampleString> {
  throw new Exception();
}

function test(): ExampleExpression<ExampleFloat> {
  // Ensure that we still get some errors
  $et = 1
    |> ExampleDsl`${ foo($$) |> bar($$) |> bar($$) }`;

  return $et;
}
