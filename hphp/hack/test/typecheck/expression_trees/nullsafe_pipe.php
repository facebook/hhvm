<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(int $_): ExampleExpression<ExampleInt> {
  throw new Exception();
}

function bar(ExampleExpression<ExampleInt> $x): ExampleExpression<ExampleString> {
  throw new Exception();
}

function helper_baz(ExampleContext $_):
  Awaitable<ExampleExpression<ExampleFunction<(function(ExampleString): ExampleFloat)>>>
{
  throw new Exception();
}

function test(): ?ExampleExpression<ExampleFloat> {
  $et = 1
    |?> foo($$)
    |?> ExampleDsl`${ $$ }`
    |?> bar($$)
    |?> ExampleDsl`helper_baz(${ $$ })`;

  return $et;
}
