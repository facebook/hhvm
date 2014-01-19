<?hh
function f($x) {
  $arr = (array)$x;
  uasort($arr, function($a,$b) {
    if (is_int($a) != is_int($b)) {
      if (is_int($a)) return -1;
      return 1;
    }
    if ($a < $b) return -1;
    if ($a > $b) return 1;
    return 0;
  });
  var_dump($arr);
}
function main() {
  $a1 = array("1" => 11, 1 => 22, "2" => 33, 3 => 44, 4 => 55, "a" => 66,
              "7" => 77);
  $m1 = Map {"1" => 11, 1 => 22, "2" => 33, 3 => 44, 4 => 55, "a" => 66,
             "7" => 77};
  $s1 = Set {"1", 1, "2", 3, 4, "a", "7"};
  $a2 = array(1 => 11, "3" => 22, 5 => 33, "a" => 44);
  $m2 = Map {1 => 11, "3" => 22, 5 => 33, "a" => 44};
  $s2 = Set {1, "3", 5, "a"};
  $a3 = array("0" => 11, 7 => 22);
  $m3 = Map {"0" => 11, 7 => 22};
  $s3 = Set {"0", 7};
  f(array_diff_key($a1, $a2));
  f(array_diff_key($m1, $a2));
  f(array_diff_key($s1, $a2));
  f(array_diff_key($a1, $m2));
  f(array_diff_key($m1, $m2));
  f(array_diff_key($s1, $m2));
  f(array_diff_key($a1, $s2));
  f(array_diff_key($m1, $s2));
  f(array_diff_key($s1, $s2));
  echo "========\n";
  f(array_diff_key($a1, $a2, $a3));
  f(array_diff_key($m1, $a2, $a3));
  f(array_diff_key($s1, $a2, $a3));
  f(array_diff_key($a1, $m2, $a3));
  f(array_diff_key($m1, $m2, $a3));
  f(array_diff_key($s1, $m2, $a3));
  f(array_diff_key($a1, $s2, $a3));
  f(array_diff_key($m1, $s2, $a3));
  f(array_diff_key($s1, $s2, $a3));
  f(array_diff_key($a1, $a2, $m3));
  f(array_diff_key($m1, $a2, $m3));
  f(array_diff_key($s1, $a2, $m3));
  f(array_diff_key($a1, $m2, $m3));
  f(array_diff_key($m1, $m2, $m3));
  f(array_diff_key($s1, $m2, $m3));
  f(array_diff_key($a1, $s2, $m3));
  f(array_diff_key($m1, $s2, $m3));
  f(array_diff_key($s1, $s2, $m3));
  f(array_diff_key($a1, $a2, $s3));
  f(array_diff_key($m1, $a2, $s3));
  f(array_diff_key($s1, $a2, $s3));
  f(array_diff_key($a1, $m2, $s3));
  f(array_diff_key($m1, $m2, $s3));
  f(array_diff_key($s1, $m2, $s3));
  f(array_diff_key($a1, $s2, $s3));
  f(array_diff_key($m1, $s2, $s3));
  f(array_diff_key($s1, $s2, $s3));
}
main();

