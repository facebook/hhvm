<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function lift<T>(T $_): ExampleExpression<T> {
  throw new Exception();
}

function test(): void {
  $x = 1;

  // Type check the splices regardless of what the overall expression tree is
  $_ = ExampleDsl`() ==> {
    ${lift($x + 1)};
    return;
  }`;
}
