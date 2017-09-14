<?hh

function foo($f) {
  $x = keyset[];
  if ($f) $x[] = 0;
  $x[] = 0;
  return $x;
}

var_dump(foo(false), foo(true));
