<?hh

abstract class T {
 function foo() {
 $this->test();
}
 }
class R extends T {
 function test() {
 var_dump('test');
 }
}

 <<__EntryPoint>>
function main_722() {
$obj = new R();
 $obj->test();
 $obj->foo();
}
