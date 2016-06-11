<?hh

function foo(): void {}

function bar(): void {}

function testLambdaBlock(bool $cond, bool $cond2): bool {
  $l =
    $cond
      ? () ==> {
        return false;
      }
      : () ==> {
        $cond2 ? foo() : bar();
        return true;
      };
  return $l();
}
