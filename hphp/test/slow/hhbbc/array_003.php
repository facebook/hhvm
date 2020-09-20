<?hh

function a() { return varray[1,2,3,6,12]; }
function b() { return varray[1,4,5]; }
function c($x) {
  $val = $x ? a() : b();
  return $val[0];
}
function main() {
  var_dump(c(true));
  var_dump(c(false));
}

<<__EntryPoint>>
function main_array_003() {
main();
}
