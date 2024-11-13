<?hh

function foo_readonly((readonly function(readonly int)[write_props]:readonly int) $x): void {
  $y = $x;

}
