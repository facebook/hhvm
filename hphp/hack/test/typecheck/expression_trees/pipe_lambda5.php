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
  // But lambda call arguments should
  $et = 1 |> Code`${ (($arg) ==> {
      $x = $arg |> foo($$);
      return $x;
    })($$) }`;

  return $et;
}
