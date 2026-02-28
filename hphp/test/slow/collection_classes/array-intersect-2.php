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
  $a1 = dict[42 => "0", 73 => 0, 2 => "2", 3 => 3, 4 => 4, 5 => "a", 6 => "7"];
  $m1 = Map {42 => "0", 73 => 0, 2 => "2", 3 => 3, 4 => 4, 5 => "a", 6 => "7"};
  $s1 = Set {0, "0", "2", 3, 4, "a", "7"};
  $a2 = vec[0, "3", 5, "a"];
  $m2 = Map {0 => 0, 1 => "3", 2 => 5, 3 => "a"};
  $s2 = Set {0, "3", 5, "a"};
  $a3 = vec["0", 7];
  $m3 = Map {0 => "0", 1 => 7};
  $s3 = Set {"0", 7};

  f(array_intersect($a1, $a2));
  f(array_intersect($m1, $a2));
  f(array_intersect($s1, $a2));
  f(array_intersect($a1, $m2));
  f(array_intersect($m1, $m2));
  f(array_intersect($s1, $m2));
  f(array_intersect($a1, $s2));
  f(array_intersect($m1, $s2));
  f(array_intersect($s1, $s2));
  echo "========\n";
  f(array_intersect($a1, $a2, $a3));
  f(array_intersect($m1, $a2, $a3));
  f(array_intersect($s1, $a2, $a3));
  f(array_intersect($a1, $m2, $a3));
  f(array_intersect($m1, $m2, $a3));
  f(array_intersect($s1, $m2, $a3));
  f(array_intersect($a1, $s2, $a3));
  f(array_intersect($m1, $s2, $a3));
  f(array_intersect($s1, $s2, $a3));
  f(array_intersect($a1, $a2, $m3));
  f(array_intersect($m1, $a2, $m3));
  f(array_intersect($s1, $a2, $m3));
  f(array_intersect($a1, $m2, $m3));
  f(array_intersect($m1, $m2, $m3));
  f(array_intersect($s1, $m2, $m3));
  f(array_intersect($a1, $s2, $m3));
  f(array_intersect($m1, $s2, $m3));
  f(array_intersect($s1, $s2, $m3));
  f(array_intersect($a1, $a2, $s3));
  f(array_intersect($m1, $a2, $s3));
  f(array_intersect($s1, $a2, $s3));
  f(array_intersect($a1, $m2, $s3));
  f(array_intersect($m1, $m2, $s3));
  f(array_intersect($s1, $m2, $s3));
  f(array_intersect($a1, $s2, $s3));
  f(array_intersect($m1, $s2, $s3));
  f(array_intersect($s1, $s2, $s3));
}


<<__EntryPoint>>
function main_array_intersect_2() :mixed{
main();
}
