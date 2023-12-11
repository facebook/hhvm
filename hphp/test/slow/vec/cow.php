<?hh

function cow(): (vec<int>, vec<int>, vec<int>) {
  $v1 = vec[1,2,3];
  $v1[] = 4; // now it's not static, and it has room for 3 more items
  $v3 = $v2 = $v1;
  $v1[] = 5;
  $v1[] = 6;
  $v2[] = 7;
  $v2[] = 8;
  return vec[$v1, $v2, $v3];
}

function main() :mixed{
  echo "---- profiling ----\n";
  var_dump(cow());
  cow();
  echo "---- pgo ----\n";
  var_dump(cow());
}


<<__EntryPoint>>
function main_cow() :mixed{
main();
}
