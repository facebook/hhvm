<?hh

function a() { return varray[]; }
function main() {
  $x = a();
  return $x + varray[1,2,3];
}

<<__EntryPoint>>
function main_empty_array_009() {
var_dump(main());
}
