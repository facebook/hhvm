<?hh

function plusOne($x) :mixed{ return $x + 1; }
function multiply($x, $y) :mixed{
  if (is_null($x)) $x = -1;
  if (is_null($y)) $y = -1;
  return $x * $y;
}

function test_mapping() :mixed{
  echo "* ", __FUNCTION__, " *\n";
  var_dump(array_map(plusOne<>, Vector {3, 5, 7}));
  var_dump(array_map(plusOne<>, Map {2 => 0, 5 => 2, 6 => 4}));
  var_dump(array_map(plusOne<>, Set {3}));
  var_dump(array_map(plusOne<>, Pair {11, 22}));
  echo "========\n";
  var_dump(array_map(multiply<>, dict[2 => 0, 4 => 2, 6 => 4], Vector {3, 5}));
  var_dump(array_map(multiply<>, Map {2 => 0, 4 => 2}, vec[3, 5, 7]));
  var_dump(array_map(multiply<>, Map {4 => 2}, Set {3}));
  var_dump(array_map(multiply<>, Pair {11, 22}, Pair {33, 44}));
  echo "========\n";
  var_dump(array_map(null, Vector {3, 5, 7}));
  var_dump(array_map(null, Map {2 => 0, 4 => 2, 6 => 4}));
  var_dump(array_map(null, Set {3}));
  var_dump(array_map(null, Pair {11, 22}));
  echo "========\n";
  var_dump(array_map(null, dict[2 => 0, 4 => 2, 6 => 4], Vector {3, 5}));
  var_dump(array_map(null, Map {2 => 0, 4 => 2}, vec[3, 5, 7]));
  var_dump(array_map(null, Map {4 => 2}, Set {3}));
  var_dump(array_map(null, Pair {11, 22}, Pair {33, 44}));
  echo "\n";
}

function map_for_key_conversion($collection) :mixed{
  echo "========\n";
  // serialization and not var_dump preserves whether keys are ints or strings
  var_dump($collection);
  $arr = array_map($x ==> $x, $collection);
  $first_converted = array_map($x ==> $x, darray($collection));
  var_dump($arr === $first_converted);
  echo serialize($arr), "\n";
  var_dump(count($arr) === count($collection));
}

function test_key_conversion() :mixed{
  map_for_key_conversion(Vector {'3', '5', '7'});
  map_for_key_conversion(Map {'2' => '0', '5' => '2', '6' => '4'});
  map_for_key_conversion(ImmMap {'1' => 'a', 1 => 'b'});
  map_for_key_conversion(Set {'3'});
  map_for_key_conversion(ImmSet {'3', 3});
  map_for_key_conversion(Pair {'11', '22'});
}

function main() :mixed{
  test_mapping();
  test_key_conversion();
}

<<__EntryPoint>>
function main_array_map() :mixed{
error_reporting(0);
main();
}
