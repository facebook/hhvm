<?hh

function fooo(
  shape(
    ?'a' => int,
  ) $x,
): void {
  if (!Shapes::keyExists($x, 'a')) {
    $x['a'] ?? null;
  }
}

function foo(
  shape(
    ?'a' => int,
  ) $x,
): shape(
  ?'a' => string,
) {
  if (Shapes::keyExists($x, 'a')) {
    $x['a'] = (string)$x['a'];
  }
  return $x;
}
