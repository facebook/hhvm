<?hh

function main() {
  $a = miarray();
  $a[0] = 1;
  $a[1] = 0;
  sort($a);
  var_dump($a);
}

main();
