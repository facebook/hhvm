<?hh

type my_shape = shape(
  'x' => int,
  'y' => bool,
);

function foo(bool $cond): my_shape {
  $s = shape('x' => 0);
  if ($cond) {
    $s['y'] = true;
  }
  return $s;
}
