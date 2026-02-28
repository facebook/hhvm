<?hh

class A {
  public static function who() :mixed{
    echo "A: " . __CLASS__;
  }
  use T;
}
trait T {
  public static function test() :mixed{
    static::who();
 // Here comes Late Static Bindings
  }
}
class B extends A {
  public static function who() :mixed{
    echo "B: " . __CLASS__;
  }
}
<<__EntryPoint>> function main(): void {
B::test();
}
