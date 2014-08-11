<?hh

function main() {
  $a = miarray();
  $a[1] = 2;
  $a[100] = 200;
  $b = array(100 => "sup", "sup" => 100);
  $c = array_diff_key($a,
                      $b);
  var_dump($a);
  var_dump($b);
  var_dump($c);
}

main();
