<?hh

function a() { return varray[]; }
function main() {
  $x = a();
  $x['heh'] = 2;
  return $x;
}

<<__EntryPoint>>
function main_empty_array_002() {
var_dump(main());
}
