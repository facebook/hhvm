<?hh

function cowPlusEq($arr) {
  $arr += array(0,1,2,3);
  return $arr;
}

function main() {
  $a = miarray();
  $a[1] = "abcd";
  $a[0] = "efgh";
  $a += array("foo" => "bar");

  $b = miarray();
  $c = miarray();
  $b[0] = 0;
  $c[1] = 1;
  $b += $c;

  $d = miarray();
  $e = array("foo" => "bar");
  $d[1] = "foo";
  $e += $d;

  $f = miarray();
  $f[100] = "sup";
  $f += array(0 => 1);

  $g = miarray();
  cowPlusEq($g);
  $g += array(0,1,2,3);
  var_dump($g);
}

main();
