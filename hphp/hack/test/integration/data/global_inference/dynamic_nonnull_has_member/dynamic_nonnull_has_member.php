<?hh

function foo(): void {
  $v = Vector {};
  $x = $v[0];
  if ($x is dynamic && $x is nonnull) {
    $x->x;
  }
}
