<?hh // strict

type Sopt = shape (?'a' => int);
type S = shape ('a' => int, ...);

function f(Sopt $s): S {
  if (Shapes::idx($s, 'a') === null) {
    $s['a'] = 0;
  }
  return $s;
}

function g(shape(...) $s): S {
  if (Shapes::idx($s, 'a') === null) {
    $s['a'] = 0;
  }
  return $s; // error: $s is shape('a' => nonnull, ...)
}

function f2(Sopt $s): S {
  if (Shapes::idx($s, 'a') !== null) {
    return $s;
  }
  $s['a'] = 0;
  return $s;
}

function g2(shape(...) $s): S {
  if (Shapes::idx($s, 'a') !== null) {
    return $s;
  }
  $s['a'] = 0;
  return $s; // error: $s is shape('a' => nonnull, ...)
}
