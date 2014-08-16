<?hh

function main() {
  $a = varray();
  $a[0] = "abcd";
  $a[1] = "efgh";
  var_dump($a);
  $b = $a + array("foo" => "bar");
  var_dump($b);
  $b = array("foo" => "bar") + $a;
  var_dump($b);

  $c = varray();
  $d = varray();
  $c[0] = 0;
  $d[1] = 1;
  $e = $c + $d;
  var_dump($e);
}

main();
