<?hh

function f(): void {
  $a = $x ? $y = &$z + 1 : $z = &$x1 * 2;
}
