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

  f(array_keys($vector));
  f($vector->toKeysArray());
  f(array_keys($map));
  f($map->toKeysArray());
  f(array_keys($set));
  f($set->toKeysArray());
  f(array_keys($pair));
  f($pair->toKeysArray());
  echo "========\n\n";
  f(array_keys($vector, 22));
  f(array_keys($vector, '22'));
  f(array_keys($map, 22));
  f(array_keys($map, '22'));
  f(array_keys($set, 22));
  f(array_keys($set, '22'));
  f(array_keys($pair, 22));
  f(array_keys($pair, '22'));
  echo "========\n\n";
  f(array_keys($vector, 22, false));
  f(array_keys($vector, '22', false));
  f(array_keys($map, 22, false));
  f(array_keys($map, '22', false));
  f(array_keys($set, 22, false));
  f(array_keys($set, '22', false));
  f(array_keys($pair, 22, false));
  f(array_keys($pair, '22', false));
  echo "========\n\n";
  f(array_keys($vector, 22, true));
  f(array_keys($vector, '22', true));
  f(array_keys($map, 22, true));
  f(array_keys($map, '22', true));
  f(array_keys($set, 22, true));
  f(array_keys($set, '22', true));
  f(array_keys($pair, 22, true));
  f(array_keys($pair, '22', true));
}
main();

