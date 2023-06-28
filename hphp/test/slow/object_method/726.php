<?hh

abstract class T {
 abstract function test():mixed;
 }
 class R extends T {
 function test() :mixed{
 var_dump('test');
 }
}

 <<__EntryPoint>>
function main_726() :mixed{
$obj = 1;
 $obj = new R();
 $obj->test();
}
