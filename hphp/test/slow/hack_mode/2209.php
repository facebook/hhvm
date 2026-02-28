<?hh
function foo(<<__Soft>> int $x):mixed{
 echo 1;
 }

 <<__EntryPoint>>
function main_2209() :mixed{
foo('hi');
}
