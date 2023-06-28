<?hh
class A {
}
function h() :mixed{
 var_dump('errored');
}


<<__EntryPoint>>
function main_1500() :mixed{
set_error_handler(h<>);
 $obj = new A;
 $obj->foo(var_dump('123'));
var_dump('end');
}
