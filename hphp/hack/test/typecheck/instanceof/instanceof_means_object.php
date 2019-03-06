<?hh // partial

function f($x) {
  $name = g();
  if ($x instanceof $name) {
    takes_string($x);
  }
}

function takes_string(string $arg): void {}
function g() {}
