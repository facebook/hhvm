<?hh

function cow_unshift($arr) {
  return array_unshift($arr, 1, 2, 3);
}

function main() {
  $a = msarray();
  array_unshift($a, 1, 2, 3);
  $a[] = "no warning";

  $a = msarray();
  cow_unshift($a);
  $a[] = "warning";
}

main();
