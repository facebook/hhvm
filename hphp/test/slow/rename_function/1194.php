<?hh

function err($code,$msg) :mixed{
 var_dump($code,$msg);
 }
function test1() :mixed{
}
function test2() :mixed{
}

<<__EntryPoint>>
function main_1194() :mixed{
set_error_handler(err<>);
fb_rename_function('test1', 'test3');
fb_rename_function('test2', 'test1');
fb_rename_function('test1', 'test2');
fb_rename_function('test3', 'test1');
}
