<?hh

function test($m1, $m2) :mixed{
  return vec[$m1 == $m2];
}

function foo($m1, $m2) :mixed{
  $a = test($m1, $m2);
  return $a[0];
}


<<__EntryPoint>>
function main_widen_bool() :mixed{
var_dump(foo(11, 11));
}
