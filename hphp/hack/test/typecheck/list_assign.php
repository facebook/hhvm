<?hh

function test<T as Vector<int>>(Vector<int> $vec, T $list): (int, int) {
  list($x, $y) = $vec;
  hh_show($x);
  hh_show($y);

  list($i1, $i2) = $list;
  hh_show($i1);
  hh_show($i2);

  return tuple($i1, $i2);
}
