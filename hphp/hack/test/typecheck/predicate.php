<?hh

function takes_float(float $x): void {}

function test(mixed $arg): void {
  if ($arg is float) {
    takes_float($arg);
  }
}
