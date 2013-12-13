<?hh
function plusOne($x) { return $x + 1; }
function multiply($x, $y) {
  if (is_null($x)) $x = -1;
  if (is_null($y)) $y = -1;
  return $x * $y;
}
var_dump(array_map('plusOne', Vector {3, 5, 7}));
var_dump(array_map('plusOne', Map {4 => 2}));
var_dump(array_map('plusOne', StableMap {2 => 0, 5 => 2, 6 => 4}));
var_dump(array_map('plusOne', Set {3}));
var_dump(array_map('plusOne', Pair {11, 22}));
echo "========\n";
var_dump(array_map('multiply', array(2 => 0, 4 => 2, 6 => 4), Vector {3, 5}));
var_dump(array_map('multiply', StableMap {2 => 0, 4 => 2}, array(3, 5, 7)));
var_dump(array_map('multiply', Map {4 => 2}, Set {3}));
var_dump(array_map('multiply', Pair {11, 22}, Pair {33, 44}));
echo "========\n";
var_dump(array_map(null, Vector {3, 5, 7}));
var_dump(array_map(null, Map {4 => 2}));
var_dump(array_map(null, StableMap {2 => 0, 4 => 2, 6 => 4}));
var_dump(array_map(null, Set {3}));
var_dump(array_map(null, Pair {11, 22}));
echo "========\n";
var_dump(array_map(null, array(2 => 0, 4 => 2, 6 => 4), Vector {3, 5}));
var_dump(array_map(null, StableMap {2 => 0, 4 => 2}, array(3, 5, 7)));
var_dump(array_map(null, Map {4 => 2}, Set {3}));
var_dump(array_map(null, Pair {11, 22}, Pair {33, 44}));

