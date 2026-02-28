<?hh

class A {
 public static $a = vec['a', 'b'];
 public static function test() :mixed{
 self::$a[] = 'c';
 var_dump(self::$a);
}
 }

 <<__EntryPoint>>
function main_1245() :mixed{
A::test();
}
