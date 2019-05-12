<?hh

abstract class Foo {
  abstract public static function who();
  public static function test() { self::who(); }
}
class Bar extends Foo {
  public static function who() { return 'Bar'; }
}

<<__EntryPoint>> function main(): void {
  $bar = new Bar();
  echo $bar::test();
}
