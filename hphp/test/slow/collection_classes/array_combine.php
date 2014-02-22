<?hh
function dump($x) {
  var_dump(is_array($x));
  var_dump($x);
}
function main() {
  $array1 = array(11, 22, 33, '22');
  $array2 = array('a', 'b', 'c', 'd');
  $vector1 = Vector {11, 22, 33, '22'};
  $vector2 = Vector {'a', 'b', 'c', 'd'};
  $map1 = Map {0 => 11, 1 => 22, 2 => 33, 3 => '22'};
  $map2 = Map {0 => 'a', 1 => 'b', 2 => 'c', 3 => 'd'};
  $set1 = Set {33, 11, '22', 22};
  $set2 = Set {'d', 'a', 'b', 'c'};

  dump(array_combine($array1, $array2));
  dump(array_combine($array1, $vector2));
  dump(array_combine($array1, $map2));
  dump(array_combine($array1, $set2));
  echo "========\n";
  dump(array_combine($vector1, $array2));
  dump(array_combine($vector1, $vector2));
  dump(array_combine($vector1, $map2));
  dump(array_combine($vector1, $set2));
  echo "========\n";
  dump(array_combine($map1, $array2));
  dump(array_combine($map1, $vector2));
  dump(array_combine($map1, $map2));
  dump(array_combine($map1, $set2));
  echo "========\n";
  dump(array_combine($set1, $array2));
  dump(array_combine($set1, $vector2));
  dump(array_combine($set1, $map2));
  dump(array_combine($set1, $set2));

  echo "\n\n========\n\n\n";

  dump(array_combine($array2, $array1));
  dump(array_combine($array2, $vector1));
  dump(array_combine($array2, $map1));
  dump(array_combine($array2, $set1));
  echo "========\n";
  dump(array_combine($vector2, $array1));
  dump(array_combine($vector2, $vector1));
  dump(array_combine($vector2, $map1));
  dump(array_combine($vector2, $set1));
  echo "========\n";
  dump(array_combine($map2, $array1));
  dump(array_combine($map2, $vector1));
  dump(array_combine($map2, $map1));
  dump(array_combine($map2, $set1));
  echo "========\n";
  dump(array_combine($set2, $array1));
  dump(array_combine($set2, $vector1));
  dump(array_combine($set2, $map1));
  dump(array_combine($set2, $set1));

  echo "\n\n========\n\n\n";

  $array = array('a', 1);
  $pair = Pair {'b', 2};
  dump(array_combine($array, $array));
  dump(array_combine($array, $pair));
  dump(array_combine($pair, $pair));
  dump(array_combine($pair, $pair));
}
main();

