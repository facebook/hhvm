<?hh
function h() {
 var_dump('errored');
}


<<__EntryPoint>>
function main_1499() {
set_error_handler(fun('h'));
foo(var_dump('123'));
var_dump('end');
}
