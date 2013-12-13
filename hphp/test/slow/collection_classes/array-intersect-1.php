<?hh
function main() {
  $a1 = array(42 => "0", 73 => 0, 2 => "2", 3 => 3, 4 => 4, 5 => "a", 6 => "7");
  $v1 = Vector {"0", 0, "2", 3, 4, "a", "7"};
  $m1 = StableMap {42 => "0", 73 => 0, 2 => "2", 3 => 3, 4 => 4, 5 => "a",
                   6 => "7"};
  $a2 = array(0, "3", 5, "a");
  $v2 = Vector {0, "3", 5, "a"};
  $m2 = StableMap {0 => 0, 1 => "3", 2 => 5, 3 => "a"};
  $a3 = array("0", 7);
  $v3 = Vector {"0", 7};
  $m3 = StableMap {0 => "0", 1 => 7};
  var_dump(array_intersect($a1, $a2));
  var_dump(array_intersect($v1, $a2));
  var_dump(array_intersect($m1, $a2));
  var_dump(array_intersect($a1, $v2));
  var_dump(array_intersect($v1, $v2));
  var_dump(array_intersect($m1, $v2));
  var_dump(array_intersect($a1, $m2));
  var_dump(array_intersect($v1, $m2));
  var_dump(array_intersect($m1, $m2));
  echo "========\n";
  var_dump(array_intersect($a1, $a2, $a3));
  var_dump(array_intersect($v1, $a2, $a3));
  var_dump(array_intersect($m1, $a2, $a3));
  var_dump(array_intersect($a1, $v2, $a3));
  var_dump(array_intersect($v1, $v2, $a3));
  var_dump(array_intersect($m1, $v2, $a3));
  var_dump(array_intersect($a1, $m2, $a3));
  var_dump(array_intersect($v1, $m2, $a3));
  var_dump(array_intersect($m1, $m2, $a3));
  var_dump(array_intersect($a1, $a2, $v3));
  var_dump(array_intersect($v1, $a2, $v3));
  var_dump(array_intersect($m1, $a2, $v3));
  var_dump(array_intersect($a1, $v2, $v3));
  var_dump(array_intersect($v1, $v2, $v3));
  var_dump(array_intersect($m1, $v2, $v3));
  var_dump(array_intersect($a1, $m2, $v3));
  var_dump(array_intersect($v1, $m2, $v3));
  var_dump(array_intersect($m1, $m2, $v3));
  var_dump(array_intersect($a1, $a2, $m3));
  var_dump(array_intersect($v1, $a2, $m3));
  var_dump(array_intersect($m1, $a2, $m3));
  var_dump(array_intersect($a1, $v2, $m3));
  var_dump(array_intersect($v1, $v2, $m3));
  var_dump(array_intersect($m1, $v2, $m3));
  var_dump(array_intersect($a1, $m2, $m3));
  var_dump(array_intersect($v1, $m2, $m3));
  var_dump(array_intersect($m1, $m2, $m3));
}
main();

