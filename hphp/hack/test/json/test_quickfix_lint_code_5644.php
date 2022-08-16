<?hh

function foo1(int $x): void {
  if ($x > 3 && true) {
    return;
  } //should provide quickfixes
}

function foo2(int $x): void {
  if (true || $x > 3) {
    return;
  } //should provide quickfixes
}

function foo3(int $x): void {
  if ($x > 3 && true || $x < 2) {
    return;
  } //should provide quickfixes
}

function foo4(int $x): void {
  if ($x > 3 || $x < 2 && $x > 3 || false || $x < 2 && $x < 4) {
    return;
  } //should provide quickfixes
}
