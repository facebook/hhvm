<?hh

function a() :mixed{ return vec[]; }
function main() :mixed{
  $x = a();
  $x[] = 2;
  return $x;
}


<<__EntryPoint>>
function main_empty_array_001() :mixed{
var_dump(main());
}
