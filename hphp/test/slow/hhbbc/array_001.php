<?hh

function a() { return array(1,2,3); }
function b() { return array(1,4,5); }
function c($x) {
  $val = $x ? a() : b();
  return $val[0];
}
function main() {
  var_dump(c(true));
  var_dump(c(false));
}

<<__EntryPoint>>
function main_array_001() {
main();
}
