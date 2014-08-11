<?hh

function cow_idx($copy) {
  return idx($copy, '10');
}

function main() {
  $arr = miarray();
  $arr[10] = 100;
  $res = idx($arr, '10');
  $arr[] = "no warning";

  $arr = miarray();
  $arr[10] = 100;
  $res = cow_idx($arr);
  $arr[] = "warning";
}

main();
