<?hh

function fill <Tk as arraykey, Tv> (
  Container<Tk> $keys,
  Tv $value,
): darray<Tk, Tv> {
  return dict[];
}

function test(int $a, int $b): void {
  $u = vec[];
  $v = fill($u, false);

  $v[$a] ??= dict[];
  $v[$a]['a'] ??= dict[];
  $v[$a]['a'][$b] ??= dict[];
  $v[$a]['a'][$b]['b'] ??= dict[];

  $v[$a] ??= dict[];
  $v[$a]['a'] ??= dict[];
  $v[$a]['a'][$b] ??= dict[];
  $v[$a]['a'][$b]['b'] ??= dict[];

  // make sure the type of $v hasn't grown out of control
  hh_show($v);
}
