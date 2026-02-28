<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(int $_): ExampleExpression<ExampleInt> {
  throw new Exception();
}

function bar(ExampleExpression<ExampleInt> $x): ExampleExpression<ExampleString> {
  throw new Exception();
}

function helper_baz(ExampleContext $_):
  Awaitable<ExampleExpression<(function(ExampleString): ExampleFloat)>>
{
  throw new Exception();
}

function test(): ExampleExpression<ExampleInt> {
  // PHP lambda bodies should not be traversed to find $$
  // But PHP lambda call arguments should
  $et = 1 |> ExampleDsl`${ (function($arg) {
      $x = $arg |> foo($$);
      return $x;
    })($$) }`;

  return $et;
}
