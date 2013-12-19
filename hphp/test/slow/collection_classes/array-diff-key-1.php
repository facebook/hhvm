<?hh
function main() {
  $a1 = array("1" => 11, 1 => 22, "2" => 33, 3 => 44, 4 => 55, "a" => 66,
              "7" => 77);
  $v1 = Vector {"1", 1, "2", 3, 4, "a", "7"};
  $m1 = StableMap {"1" => 11, 1 => 22, "2" => 33, 3 => 44, 4 => 55, "a" => 66,
                   "7" => 77};
  $a2 = array(1 => 11, "3" => 22, 5 => 33, "a" => 44);
  $v2 = Vector {1, "3", 5, "a"};
  $m2 = StableMap {1 => 11, "3" => 22, 5 => 33, "a" => 44};
  $a3 = array("0" => 11, 7 => 22);
  $v3 = Vector {"0", 7};
  $m3 = StableMap {"0" => 11, 7 => 22};
  var_dump(array_diff_key($a1, $a2));
  var_dump(array_diff_key($v1, $a2));
  var_dump(array_diff_key($m1, $a2));
  var_dump(array_diff_key($a1, $v2));
  var_dump(array_diff_key($v1, $v2));
  var_dump(array_diff_key($m1, $v2));
  var_dump(array_diff_key($a1, $m2));
  var_dump(array_diff_key($v1, $m2));
  var_dump(array_diff_key($m1, $m2));
  echo "========\n";
  var_dump(array_diff_key($a1, $a2, $a3));
  var_dump(array_diff_key($v1, $a2, $a3));
  var_dump(array_diff_key($m1, $a2, $a3));
  var_dump(array_diff_key($a1, $v2, $a3));
  var_dump(array_diff_key($v1, $v2, $a3));
  var_dump(array_diff_key($m1, $v2, $a3));
  var_dump(array_diff_key($a1, $m2, $a3));
  var_dump(array_diff_key($v1, $m2, $a3));
  var_dump(array_diff_key($m1, $m2, $a3));
  var_dump(array_diff_key($a1, $a2, $v3));
  var_dump(array_diff_key($v1, $a2, $v3));
  var_dump(array_diff_key($m1, $a2, $v3));
  var_dump(array_diff_key($a1, $v2, $v3));
  var_dump(array_diff_key($v1, $v2, $v3));
  var_dump(array_diff_key($m1, $v2, $v3));
  var_dump(array_diff_key($a1, $m2, $v3));
  var_dump(array_diff_key($v1, $m2, $v3));
  var_dump(array_diff_key($m1, $m2, $v3));
  var_dump(array_diff_key($a1, $a2, $m3));
  var_dump(array_diff_key($v1, $a2, $m3));
  var_dump(array_diff_key($m1, $a2, $m3));
  var_dump(array_diff_key($a1, $v2, $m3));
  var_dump(array_diff_key($v1, $v2, $m3));
  var_dump(array_diff_key($m1, $v2, $m3));
  var_dump(array_diff_key($a1, $m2, $m3));
  var_dump(array_diff_key($v1, $m2, $m3));
  var_dump(array_diff_key($m1, $m2, $m3));
}
main();

