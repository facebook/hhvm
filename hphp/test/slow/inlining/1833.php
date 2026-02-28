<?hh

function g($a) :mixed{
 return $a ? vec[1,2,3] : 'foo';
 }
function f($a) :mixed{
 return g($a);
 }
function test($a) :mixed{
  $f = f($a);
  return $f[0];
}

<<__EntryPoint>>
function main_1833() :mixed{
var_dump(test(1));
}
