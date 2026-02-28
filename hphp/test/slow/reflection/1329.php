<?hh

class A {
 public static function test() :mixed{
 print 'ok';
}
}

<<__EntryPoint>>
function main_1329() :mixed{
var_dump(is_callable('A::test'));
var_dump(function_exists('A::test'));
}
