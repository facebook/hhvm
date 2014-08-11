<?hh

function cow_shift($arr) {
  return array_shift($arr);
}

function main() {
  $a = msarray();
  $a['foo'] = 10;
  $b = array_shift($a);
  $a[] = "no warning";

  $a = msarray();
  $a['foo'] = 10;
  $b = cow_shift($a);
  $a[] = "warning";
}

main();
