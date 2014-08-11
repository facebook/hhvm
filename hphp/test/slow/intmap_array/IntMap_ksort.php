<?hh

function main() {
  $a = miarray();
  $a[1] = 0;
  $a[0] = 1;
  var_dump($a);
  ksort($a);
  var_dump($a);
}

main();
