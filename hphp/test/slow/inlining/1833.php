<?hh

function g($a) {
 return $a ? varray[1,2,3] : 'foo';
 }
function f($a) {
 return g($a);
 }
function test($a) {
  $f = f($a);
  return $f[0];
}

<<__EntryPoint>>
function main_1833() {
var_dump(test(1));
}
