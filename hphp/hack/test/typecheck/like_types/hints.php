<?hh

function f(
  ~int $a,
  ~?int $b,
  ?~?int $c,
  vec<~int> $d,
  @int $e,
  ~@int $f,
  ~~int $g,
): void {
  hh_show($a);
  hh_show($b);
  hh_show($c);
  hh_show($d);
  hh_show($e);
  hh_show($f);
  hh_show($g);
}
