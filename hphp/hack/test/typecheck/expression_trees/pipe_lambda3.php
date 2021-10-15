<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(int $_): ExprTree<Code, Code::TAst, ExampleInt> {
  throw new Exception();
}

function bar(ExprTree<Code, Code::TAst, ExampleInt> $x): ExprTree<Code, Code::TAst, ExampleString> {
  throw new Exception();
}

function helper_baz(ExampleContext $_):
  Awaitable<ExprTree<Code, Code::TAst, (function(ExampleString): ExampleFloat)>>
{
  throw new Exception();
}

function test(): ExprTree<Code, Code::TAst, ExampleInt> {
  // Lambda bodies should not be traversed to find $$
  $et = 1 |> Code`${ (function() {
      $x = 1 |> foo($$);
      return $x;
    })() }`;

  return $et;
}
