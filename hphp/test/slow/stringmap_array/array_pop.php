<?hh

function cow_pop($arr) {
  return array_pop($arr);
}

function main() {
  $a = msarray();
  $a['str'] = 10;
  array_pop($a);
  $a[] = "no warning";

  $a = msarray();
  $a['str'] = 10;
  cow_pop($a);
  $a[] = "warning";
}

main();
