<?hh
function f() {
  $arr2 = darray[0 => 'a'];
  $arr1 = darray[0 => 'a', 1 => 'b'];
  unset($arr1[0]);
  reset(inout $arr1);
  reset(inout $arr2);
  var_dump(current($arr1));
  $arr1 += $arr2;
  var_dump(current($arr1));
}


<<__EntryPoint>>
function main_internal_cursor_bug() {
f();
}
