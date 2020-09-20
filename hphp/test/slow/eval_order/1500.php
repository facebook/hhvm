<?hh
class A {
}
function h() {
 var_dump('errored');
}


<<__EntryPoint>>
function main_1500() {
set_error_handler(fun('h'));
 $obj = new A;
 $obj->foo(var_dump('123'));
var_dump('end');
}
