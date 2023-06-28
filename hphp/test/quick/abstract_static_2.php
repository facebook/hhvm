<?hh

abstract class Foo {
  abstract public static function who():mixed;
  public static function test() :mixed{ self::who(); }
}
class Bar extends Foo {
  public static function who() :mixed{ return 'Bar'; }
}

<<__EntryPoint>> function main(): void {
  $bar = new Bar();
  echo $bar::test();
}
