<?hh

function main() {
  $a = miarray();
  $a[0] = "efgh";
  $a[1] = "abcd";
  var_dump($a);
  asort($a);
  var_dump($a);
}

main();
