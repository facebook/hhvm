<?hh
abstract class Base {
  public static function foo() :mixed{
    $a = 2;
    static::$x;
  }
}
class Derived extends Base {
  public static $x;
}
<<__EntryPoint>> function main(): void {
$a = new Derived();
$a::foo();
echo "Done\n";
}
