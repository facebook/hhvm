<?hh

function a() :mixed{ return dict[]; }
function main() :mixed{
  $x = a();
  $x[12] = 2;
  return $x;
}

<<__EntryPoint>>
function main_empty_array_004() :mixed{
var_dump(main());
}
