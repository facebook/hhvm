<?hh
class Foo {
 public function __construct() {
  eval("class Bar extends Foo {}");
 }
 <<__LSB>> public static $i = 0;
 public static function test() :mixed{
  var_dump(++static::$i);
 }
}
<<__EntryPoint>> function main(): void {
Foo::test();
new Foo;
Foo::test();

/**
 * function_add_ref() makes a clone of static variables for inherited functions, so $i in Bar::test gets initial value 1
 */
Bar::test();
Bar::test();
}
