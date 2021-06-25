<?hh

interface B {}
interface C {}
function test(vec<B> $v): void {
  $w = Vec_map($v, $x ==> $x as ?C);
  hh_force_solve();
  hh_show($w);
}

function Vec_map<Tv1, Tv2>(
  Traversable<Tv1> $traversable,
  (function(Tv1): Tv2) $value_func,
): vec<Tv2> {
  return vec[];
}
