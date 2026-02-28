<?hh

abstract class T {
 function foo() :mixed{
 $this->test();
}
 }
class R extends T {
 function test() :mixed{
 var_dump('test');
 }
}

 <<__EntryPoint>>
function main_722() :mixed{
$obj = new R();
 $obj->test();
 $obj->foo();
}
