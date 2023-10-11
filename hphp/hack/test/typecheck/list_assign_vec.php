<?hh

function test<T as vec<int>>(vec<int> $vec, T $list): (int, int) {
  list($x, $y) = $vec;
  hh_show($x);
  hh_show($y);

  list($a, $b) = vec['asterix', 'obelix'];
  hh_show($a);
  hh_show($b);

  list($i1, $i2) = $list;
  hh_show($i1);
  hh_show($i2);

  return tuple($i1, $i2);
}
