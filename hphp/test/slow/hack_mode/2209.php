<?hh
function foo(<<__Soft>> int $x){
 echo 1;
 }

 <<__EntryPoint>>
function main_2209() {
foo('hi');
}
