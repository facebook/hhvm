<?hh

function poly<T as arraykey>(T $x, int $y, string $z): arraykey {
  if($x is int) {
    return $x+$y;
  } else {
    return $x.'+'.$z;
  }
}

function call_generic(arraykey $x): void {
  $f = poly<>;
  $_ = $f($x, 1, "2");
  $_ = $f(1, 1, "2");
  $_ = $f('1', 1, "2");
}
