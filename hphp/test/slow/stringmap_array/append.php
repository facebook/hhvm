<?hh

function cow_append($arr) {
  $arr[] = "warning";
}

function main() {
  $a = msarray();
  $a[] = "warning";
  $a[] = "no warning";

  $a = msarray();
  cow_append($a);
  $a[] = "warning";
}

main();
