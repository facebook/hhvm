<?hh

function takes_int(int $_): void {}

function f(mixed $m): void {
  if ($m is C) { // C does not exist.
    takes_int($m); // Terr works in place of any type.
  }
}
