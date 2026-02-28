<?hh

trait T1 {
  static function hello() :mixed{
    echo "Hello ";
  }
}
trait T2 {
  use T1;
  static function world() :mixed{
    echo "World!\n";
  }
}
class C {
  use T2;
  static function p() :mixed{
    self::hello();
    self::world();
  }
}
<<__EntryPoint>> function main(): void {
C::p();
}
