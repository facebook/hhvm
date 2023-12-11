<?hh

function a() :mixed{ return dict[]; }
function main() :mixed{
  $x = a();
  $x['heh'] = 2;
  return $x;
}

<<__EntryPoint>>
function main_empty_array_002() :mixed{
var_dump(main());
}
