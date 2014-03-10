<?hh
function f($x) {
  var_dump(is_array($x));
  foreach ($x as $k => $_) {
    var_dump($k);
  }
  usort($x, function($x,$y) {
    if (is_int($x) != is_int($y)) {
      if (is_int($x)) return -1;
      return 1;
    }
    if ($x < $y) return -1;
    if ($x > $y) return 1;
    return 0;
  });
  var_dump($x);
}
function main() {
  $vector = Vector {11, 22, 33, '22'};
  $map = Map {'a' => 11, 'b' => 22, 'c' => 33, 'd' => '22'};
  $set = Set {11, 22, 33, '22'};
  $pair = Pair {22, '22'};

  f(array_values($vector));
  f($vector->toValuesArray());
  f(array_values($map));
  f($map->toValuesArray());
  f(array_values($set));
  f($set->toValuesArray());
  f(array_values($pair));
  f($pair->toValuesArray());
}
main();

