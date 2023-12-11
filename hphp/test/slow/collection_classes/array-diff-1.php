<?hh
function main() :mixed{
  $a1 = vec["1", 1, "2", 3, 4, "a", "7"];
  $v1 = Vector {"1", 1, "2", 3, 4, "a", "7"};
  $m1 = Map {0 => "1", 1 => 1, 2 => "2", 3 => 3, 4 => 4, 5 => "a",
                   6 => "7"};
  $a2 = vec[1, "3", 5, "a"];
  $v2 = Vector {1, "3", 5, "a"};
  $m2 = Map {0 => 1, 1 => "3", 2 => 5, 3 => "a"};
  $a3 = vec["0", 7];
  $v3 = Vector {"0", 7};
  $m3 = Map {0 => "0", 1 => 7};
  var_dump(array_diff($a1, $a2));
  var_dump(array_diff($v1, $a2));
  var_dump(array_diff($m1, $a2));
  var_dump(array_diff($a1, $v2));
  var_dump(array_diff($v1, $v2));
  var_dump(array_diff($m1, $v2));
  var_dump(array_diff($a1, $m2));
  var_dump(array_diff($v1, $m2));
  var_dump(array_diff($m1, $m2));
  echo "========\n";
  var_dump(array_diff($a1, $a2, $a3));
  var_dump(array_diff($v1, $a2, $a3));
  var_dump(array_diff($m1, $a2, $a3));
  var_dump(array_diff($a1, $v2, $a3));
  var_dump(array_diff($v1, $v2, $a3));
  var_dump(array_diff($m1, $v2, $a3));
  var_dump(array_diff($a1, $m2, $a3));
  var_dump(array_diff($v1, $m2, $a3));
  var_dump(array_diff($m1, $m2, $a3));
  var_dump(array_diff($a1, $a2, $v3));
  var_dump(array_diff($v1, $a2, $v3));
  var_dump(array_diff($m1, $a2, $v3));
  var_dump(array_diff($a1, $v2, $v3));
  var_dump(array_diff($v1, $v2, $v3));
  var_dump(array_diff($m1, $v2, $v3));
  var_dump(array_diff($a1, $m2, $v3));
  var_dump(array_diff($v1, $m2, $v3));
  var_dump(array_diff($m1, $m2, $v3));
  var_dump(array_diff($a1, $a2, $m3));
  var_dump(array_diff($v1, $a2, $m3));
  var_dump(array_diff($m1, $a2, $m3));
  var_dump(array_diff($a1, $v2, $m3));
  var_dump(array_diff($v1, $v2, $m3));
  var_dump(array_diff($m1, $v2, $m3));
  var_dump(array_diff($a1, $m2, $m3));
  var_dump(array_diff($v1, $m2, $m3));
  var_dump(array_diff($m1, $m2, $m3));
}


<<__EntryPoint>>
function main_array_diff_1() :mixed{
main();
}
