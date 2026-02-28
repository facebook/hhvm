<?hh

function f($x) :mixed{ return $x; }

function g($t) :mixed{
  $a = 1;
  $b = 1;
  return f($t ? $a : $b);
}


<<__EntryPoint>>
function main_dce_crash() :mixed{
var_dump(g(true));
}
