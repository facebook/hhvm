<?hh

function test() {
  $a = 1;
  $b = 2;
  $c = 3;
  $d = 4;
  $e = 5;
  $f = 6;
  $g = 7;
  $h = 8;
  $i = 9;
  $j = 10;
  $k = 11;
  __hhvm_intrinsics\launder_value($a);
  var_dump($a, $b, $c, $d, $e, $f, $g, $h, $i, $j, $k);
}

test();
