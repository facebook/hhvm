<?hh

function k1() {
  $arr = varray[0,1,2,3,4];
  reset(inout $arr);
  foreach ($arr as $v) {
    echo "val=$v\n";
  }
  var_dump(current($arr));
}

<<__EntryPoint>>
function main_495() {
k1();
}
