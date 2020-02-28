<?hh

function test1() {
 $a = varray[__FUNCTION__, __LINE__];
 return $a;
 }
function test2() {
 $a = varray[__FUNCTION__, __LINE__];
 return $a;
 }

<<__EntryPoint>>
function main_546() {
var_dump(test1());
 var_dump(test2());
}
