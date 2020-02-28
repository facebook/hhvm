<?hh

function h8() {
  $arr = varray[0,1,2,3,4];
  end(inout $arr);
  next(inout $arr);
  $arr2 = $arr;
  var_dump(current($arr2));
  var_dump(current($arr));
}

<<__EntryPoint>>
function main_257() {
h8();
}
