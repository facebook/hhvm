<?hh

function k2() {
  $arr = varray[0,1,2,3,4];
  reset(inout $arr);
  $arr2 = $arr;
  foreach ($arr as $v) {
    echo "val=$v\n";
  }
  var_dump(current($arr));
  var_dump(current($arr2));
}

<<__EntryPoint>>
function main_496() {
k2();
}
