<?hh

function cow_unset($arr) {
  unset($arr[10]);
}

function main() {
  $a = msarray();
  $a['foo'] = 1;
  $a['bar'] = 2;
  unset($a['foo']);
  var_dump($a);

  unset($a[10]); // warning

  $a = msarray();
  cow_unset($a);
  $a[] = "warning";
}

main();
