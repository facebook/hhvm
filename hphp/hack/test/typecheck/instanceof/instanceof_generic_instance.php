<?hh

function f<T>($x, T $y): T {
  if (!$x instanceof $y) {
    throw new Exception('');
  }
  hh_show($x);
  return $x;
}
