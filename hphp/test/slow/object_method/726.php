<?hh

abstract class T {
 abstract function test();
 }
 class R extends T {
 function test() {
 var_dump('test');
 }
}

 <<__EntryPoint>>
function main_726() {
$obj = 1;
 $obj = new R();
 $obj->test();
}
