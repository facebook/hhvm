<?hh

function test_varray($a) {
  var_dump(current($a));
  end(inout $a);
  $a[] = 2;
  $a[] = 3;
  var_dump(current($a));
  end(inout $a);
  var_dump(current($a));
}

function test_darray($a) {
  var_dump(current($a));
  end(inout $a);
  $a[] = 2;
  $a[] = 3;
  var_dump(current($a));
  end(inout $a);
  var_dump(current($a));
  $a[17] = 4;
  var_dump(current($a));
  $a[18] = 5;
  var_dump(current($a));
  end(inout $a);
  var_dump(current($a));
  $a[1] = 6;
  var_dump(current($a));
}

function test($a) {
  print("\n==================================\nTesting: ");
  print(json_encode($a)."\n");
  print("\ntest_varray:\n");
  test_varray(varray($a));
  print("\ntest_darray:\n");
  test_darray(darray($a));
}

<<__EntryPoint>>
function main_235() {
test(varray[1]);
test(varray[1,2,3,4,5,6,7,8,9]);
}
