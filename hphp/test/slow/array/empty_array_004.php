<?hh

function a() { return darray[]; }
function main() {
  $x = a();
  $x[12] = 2;
  return $x;
}

<<__EntryPoint>>
function main_empty_array_004() {
var_dump(main());
}
