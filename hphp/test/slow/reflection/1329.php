<?hh

class A {
 public static function test() {
 print 'ok';
}
}

<<__EntryPoint>>
function main_1329() {
var_dump(is_callable('A::test'));
var_dump(function_exists('A::test'));
}
