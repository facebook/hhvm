<?hh

function foo($v) :mixed{
  $a = dict['key' => $v];
  return $a;
}
function goo($v) :mixed{
  return $v . 1;
}

<<__EntryPoint>>
function main_1522() :mixed{
var_dump(foo('1.0'));
var_dump(foo(foo('1.0')));
var_dump(foo(goo('1.0')));
}
