<?hh

function cowPlusEq($arr) {
  $arr += array(0,1,2,3);
  return $arr;
}

function main() {
  $a = varray();
  $a[0] = "abcd";
  $a[1] = "efgh";
  $a += array("foo" => "bar");

  $b = varray();
  $c = varray();
  $b[0] = 0;
  $c[0] = 1;
  $c[1] = 2;
  $b += $c;

  $d = varray();
  $e = array("foo" => "bar");
  $d[0] = "foo";
  $d[1] = "bar";
  $d[2] = "baz";
  $e += $d;

  $f = varray();
  $f[0] = "foo";
  $f[1] = "bar";
  $f[2] = "baz";
  $f += array(0 => 1);

  $g = varray();
  cowPlusEq($g);
  $g += array(0,1,2,3);
  var_dump($g);
}

main();
