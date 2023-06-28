<?hh

abstract class T {
 abstract function test():mixed;
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
function main_725() :mixed{
$obj = new R();
 $obj->test();
 $obj->foo();
}
