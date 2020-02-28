<?hh

function h7() {
  $arr = varray[0,1,2,3,4];
  end(inout $arr);
  next(inout $arr);
  $arr2 = $arr;
  var_dump(current($arr));
  var_dump(current($arr2));
}

<<__EntryPoint>>
function main_256() {
h7();
}
