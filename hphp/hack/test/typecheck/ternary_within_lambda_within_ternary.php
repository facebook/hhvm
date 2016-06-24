<?hh

function foo(): void {
}

function bar(): void {
}

function testLambdaExpression(bool $cond, bool $cond2): void {
  $l = $cond
    ? () ==> foo()
    : () ==>
      $cond2 ? foo() : bar();
  $l();
}
