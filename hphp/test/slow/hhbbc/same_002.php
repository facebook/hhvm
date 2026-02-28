<?hh

function foo($x) :mixed{
  $a = $x ? "a" : null;
  $b = "b";

  if ($a !== $b) return $a;
  return $a . $b;
}


<<__EntryPoint>>
function main_same_002() :mixed{
var_dump(foo(true));
var_dump(foo(false));
}
