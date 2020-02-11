<?hh

function __autoload($class) {
    var_dump($class);
}
<<__EntryPoint>> function main(): void {
call_user_func(varray['foo', 'bar']);
call_user_func(varray['', 'bar']);
call_user_func(varray[$foo, 'bar']);
call_user_func(varray[$foo, '']);
}
