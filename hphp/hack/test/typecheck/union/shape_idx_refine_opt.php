<?hh // strict

type Sopt = shape (?'a' => ?int);
type S = shape ('a' => int, ...);

function f(Sopt $s): S {
  if (Shapes::idx($s, 'a') === null) {
    $s['a'] = 0;
  }
  return $s;
}

function f2(Sopt $s): S {
  if (Shapes::idx($s, 'a') !== null) {
    return $s;
  }
  $s['a'] = 0;
  return $s;
}
