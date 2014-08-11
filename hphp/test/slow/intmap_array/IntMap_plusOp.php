<?hh

function main() {
  $a = miarray();
  $a[1] = "abcd";
  $a[0] = "efgh";
  var_dump($a);
  $b = $a + array("foo" => "bar");
  var_dump($b);
  $b = array("foo" => "bar") + $a;
  var_dump($b);

  $c = miarray();
  $d = miarray();
  $c[0] = 0;
  $d[1] = 1;
  $e = $c + $d;
  var_dump($e);
}

main();
