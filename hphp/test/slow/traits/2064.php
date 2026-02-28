<?hh

trait T {
  public static function goo() :mixed{
    return static::class;
  }
  public static function foo() :mixed{
     return self::goo();
  }
}
class A {
 use T;
 }

<<__EntryPoint>>
function main_2064() :mixed{
var_dump(A::goo());
var_dump(A::foo());
}
