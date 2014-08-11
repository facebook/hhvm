<?hh

function cow_pop($arr) {
  return array_pop($arr);
}

function main() {
  $a = miarray();
  $a[10] = 10;
  $b = array_pop($a);
  $a[] = "no warning";
  var_dump($b);

  $a = miarray();
  $a[10] = 10;
  $b = cow_pop($a);
  $a[] = "warning";
  var_dump($b);
}

main();
