<?hh
class Foo {
  const string CONSTANT = 'a';

  final public static function test(): string {
    return self::CONSTANT;
  }
}
