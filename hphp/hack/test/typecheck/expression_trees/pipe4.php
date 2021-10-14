<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(int $_): ExprTree<Code, Code::TAst, ExampleInt> {
  throw new Exception();
}

function test(): ExprTree<Code, Code::TAst, ExampleFloat> {
  // Ensure that we still get some errors
  $et = 1
    |> Code`${ foo($$ + $$ - $$) |> foo($$) }`;

  return $et;
}
