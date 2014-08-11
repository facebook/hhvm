<?hh

function main() {
  $a = miarray();
  $a[1] = "abcd";
  $a[0] = "efgh";
  $b = array("foo" => "bar", 1 => "one", 2 => 600);
  $c = array_merge($a, $b);
  var_dump($c);
  $d = array_merge($b, $a);
  var_dump($d);
  $e = array_merge($a, $a);
  var_dump($e);
}

main();
