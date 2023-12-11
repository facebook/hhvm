<?hh

function a() :mixed{ return vec[1,2,3]; }
function b() :mixed{ return vec[1,4,5]; }
function c($x) :mixed{
  $val = $x ? a() : b();
  return $val[5]; // out of range
}
<<__EntryPoint>>
function main() :mixed{
  try { var_dump(c(true)); } catch (Exception $e) { echo $e->getMessage()."\n"; }
  try { var_dump(c(false)); } catch (Exception $e) { echo $e->getMessage()."\n"; }
}
