<?hh

function foo_readonly((readonly function(readonly int):readonly int) $x): void {
  $y = $x;

}
