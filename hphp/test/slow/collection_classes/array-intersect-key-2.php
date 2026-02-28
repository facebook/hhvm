<?hh
function f($x) :mixed{
  $arr = darray($x);
  uasort(inout $arr, function($a,$b) {
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
function main() :mixed{
  $a1 = dict["0" => 11, 1 => 22, "2" => 33, 3 => 44, 4 => 55, "a" => 66,
              "7" => 77];
  $m1 = Map {"0" => 11, 1 => 22, "2" => 33, 3 => 44, 4 => 55, "a" => 66,
             "7" => 77};
  $s1 = Set {"0", 1, "2", 3, 4, "a", "7"};
  $a2 = dict[0 => 11, 1 => 22, "3" => 33, 5 => 44, "a" => 55];
  $m2 = Map {0 => 11, 1 => 22, "3" => 33, 5 => 44, "a" => 55};
  $s2 = Set {0, 1, "3", 5, "a"};
  $a3 = dict["0" => 11, 1 => 22, 7 => 33];
  $m3 = Map {"0" => 11, 1 => 22, 7 => 33};
  $s3 = Set {"0", 1, 7};
  f(array_intersect_key($a1, $a2));
  f(array_intersect_key($m1, $a2));
  f(array_intersect_key($s1, $a2));
  f(array_intersect_key($a1, $m2));
  f(array_intersect_key($m1, $m2));
  f(array_intersect_key($s1, $m2));
  f(array_intersect_key($a1, $s2));
  f(array_intersect_key($m1, $s2));
  f(array_intersect_key($s1, $s2));
  echo "========\n";
  f(array_intersect_key($a1, $a2, $a3));
  f(array_intersect_key($m1, $a2, $a3));
  f(array_intersect_key($s1, $a2, $a3));
  f(array_intersect_key($a1, $m2, $a3));
  f(array_intersect_key($m1, $m2, $a3));
  f(array_intersect_key($s1, $m2, $a3));
  f(array_intersect_key($a1, $s2, $a3));
  f(array_intersect_key($m1, $s2, $a3));
  f(array_intersect_key($s1, $s2, $a3));
  f(array_intersect_key($a1, $a2, $m3));
  f(array_intersect_key($m1, $a2, $m3));
  f(array_intersect_key($s1, $a2, $m3));
  f(array_intersect_key($a1, $m2, $m3));
  f(array_intersect_key($m1, $m2, $m3));
  f(array_intersect_key($s1, $m2, $m3));
  f(array_intersect_key($a1, $s2, $m3));
  f(array_intersect_key($m1, $s2, $m3));
  f(array_intersect_key($s1, $s2, $m3));
  f(array_intersect_key($a1, $a2, $s3));
  f(array_intersect_key($m1, $a2, $s3));
  f(array_intersect_key($s1, $a2, $s3));
  f(array_intersect_key($a1, $m2, $s3));
  f(array_intersect_key($m1, $m2, $s3));
  f(array_intersect_key($s1, $m2, $s3));
  f(array_intersect_key($a1, $s2, $s3));
  f(array_intersect_key($m1, $s2, $s3));
  f(array_intersect_key($s1, $s2, $s3));
}


<<__EntryPoint>>
function main_array_intersect_key_2() :mixed{
main();
}
