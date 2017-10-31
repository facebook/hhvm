<?hh

function takes_int(int $x): void {}

function test($arg): void {
  if (is_float($arg)) {
    takes_int($arg);
  }
}
