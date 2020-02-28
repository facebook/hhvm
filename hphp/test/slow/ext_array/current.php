<?hh

function a() {
  $transport = varray["foot", "bike", "car", "plane"];
  var_dump(current($transport));
  var_dump(next(inout $transport));
  var_dump(current($transport));
  var_dump(prev(inout $transport));
  var_dump(end(inout $transport));
  var_dump(current($transport));
}

function b() {
  $arr = varray[];
  var_dump(current($arr));
}

function c() {
  $arr = varray[varray[]];
  var_dump(current($arr));
}


<<__EntryPoint>>
function main_current() {
a();
b();
c();
}
