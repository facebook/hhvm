<?hh

function g($a) {
 return $a ? varray[1,2,3] : 'foo';
 }
function f($a) {
 return g($a);
 }
function test($a) {
  $f = f($a);
  return reset(inout $f);
}

<<__EntryPoint>>
function main_1833() {
var_dump(test(1));
}
