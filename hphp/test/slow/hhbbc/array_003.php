<?hh

function a() :mixed{ return vec[1,2,3,6,12]; }
function b() :mixed{ return vec[1,4,5]; }
function c($x) :mixed{
  $val = $x ? a() : b();
  return $val[0];
}
function main() :mixed{
  var_dump(c(true));
  var_dump(c(false));
}

<<__EntryPoint>>
function main_array_003() :mixed{
main();
}
