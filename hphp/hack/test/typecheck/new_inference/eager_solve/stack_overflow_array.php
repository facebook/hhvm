<?hh // strict

function fill <Tk as arraykey, Tv> (
  Container<Tk> $keys,
  Tv $value,
): darray<Tk, Tv> {
  return darray[];
}

function test(int $a, int $b): void {
  $u = vec[];
  $v = fill($u, false);

  $v[$a] ??= darray[];
  $v[$a]['a'] ??= darray[];
  $v[$a]['a'][$b] ??= darray[];
  $v[$a]['a'][$b]['b'] ??= darray[];

  $v[$a] ??= darray[];
  $v[$a]['a'] ??= darray[];
  $v[$a]['a'][$b] ??= darray[];
  $v[$a]['a'][$b]['b'] ??= darray[];

  // make sure the type of $v hasn't grown out of control
  hh_show($v);
}
