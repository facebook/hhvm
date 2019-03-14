<?hh // partial

function takes_int(int $x): void {}

function test($arg): void {
  if ($arg is float) {
    takes_int($arg);
  }
}
