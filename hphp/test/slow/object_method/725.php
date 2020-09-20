<?hh

abstract class T {
 abstract function test();
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
function main_725() {
$obj = new R();
 $obj->test();
 $obj->foo();
}
