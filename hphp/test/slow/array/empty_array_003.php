<?hh

function a() { return varray[]; }
function main() {
  $x = a();
  $x[0] = 2;
  return $x;
}

<<__EntryPoint>>
function main_empty_array_003() {
var_dump(main());
}
