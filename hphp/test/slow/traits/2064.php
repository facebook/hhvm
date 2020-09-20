<?hh

trait T {
  public static function goo() {
    return static::class;
  }
  public static function foo() {
     return self::goo();
  }
}
class A {
 use T;
 }

<<__EntryPoint>>
function main_2064() {
var_dump(A::goo());
var_dump(A::foo());
}
