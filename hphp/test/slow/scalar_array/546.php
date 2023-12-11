<?hh

function test1() :mixed{
 $a = vec[__FUNCTION__, __LINE__];
 return $a;
 }
function test2() :mixed{
 $a = vec[__FUNCTION__, __LINE__];
 return $a;
 }

<<__EntryPoint>>
function main_546() :mixed{
var_dump(test1());
 var_dump(test2());
}
