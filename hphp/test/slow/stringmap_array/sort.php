<?hh

function cow_sort($arr) {
  sort($arr);
}

function main() {
  $a = msarray();
  $a['foo'] = 1;
  $a['bar'] = 2;

  sort($a);
  $a[] = 'no warning';

  $a = msarray();
  $a['foo'] = 1;
  $a['bar'] = 2;
  cow_sort($a);
  $a[] = "warning";
}

main();
