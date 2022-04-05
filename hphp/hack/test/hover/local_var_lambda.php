<?hh

function takes_lambda((function(int): int) $f): void {}

function foo(): void {
  takes_lambda(() ==> {
    $x = 1;
//  ^ hover-at-caret
    return $x + 1;
  });
}
