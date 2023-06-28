<?hh

function f($a) :mixed{
 $a = varray[$a];
 var_dump($a);
 }

<<__EntryPoint>>
function main_258() :mixed{
f(false);
}
