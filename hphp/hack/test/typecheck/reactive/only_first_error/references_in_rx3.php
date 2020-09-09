<?hh // partial

<<__Rx>>
function foo(): void {
  $foo = 'f';
  $z = call(&$foo); // bad
}

<<__Rx>>
function call($_) {}
