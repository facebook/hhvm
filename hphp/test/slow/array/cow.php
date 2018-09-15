<?hh

function cow(): (array<int>, array<int>, array<int>) {
  $a1 = [1,2,3];
  $a1[] = 4; // now it's not static, and it has room for 3 more items
  $a3 = $a2 = $a1;
  $a1[] = 5;
  $a1[] = 6;
  $a2[] = 7;
  $a2[] = 8;
  return [$a1, $a2, $a3];
}

function main() {
  echo "---- profiling ----\n";
  var_dump(cow());
  cow();
  echo "---- pgo ----\n";
  var_dump(cow());
}


<<__EntryPoint>>
function main_cow() {
main();
}
