<?hh

function a() { return varray[1,2,3]; }
function b() { return varray[1,4,5]; }
function c($x) {
  $val = $x ? a() : b();
  return $val[5]; // out of range
}
<<__EntryPoint>>
function main() {
  try { var_dump(c(true)); } catch (Exception $e) { echo $e->getMessage()."\n"; }
  try { var_dump(c(false)); } catch (Exception $e) { echo $e->getMessage()."\n"; }
}
