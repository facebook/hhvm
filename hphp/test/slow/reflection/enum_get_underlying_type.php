<?hh

enum Foo: int {
  BAR = 42;
}

enum Bar: int as arraykey {
  FOO = 42;
}

newtype nt = int;
enum WithNT: nt {
  FOO = 42;
}

type t = arraykey;
enum WithT: t {
  FOO = 42;
}

class FooBar extends HH\BuiltinEnum<string> {
  const BAZ = '42';
}

class Normcore {}

<<__EntryPoint>> function main(): void {
  var_dump((new ReflectionClass('Foo'))->getEnumUnderlyingType());
  var_dump((new ReflectionClass('Bar'))->getEnumUnderlyingType());
  var_dump((new ReflectionClass('WithNT'))->getEnumUnderlyingType());
  var_dump((new ReflectionClass('WithT'))->getEnumUnderlyingType());
  try {
    (new ReflectionClass(FooBar::class))->getEnumUnderlyingType();
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    (new ReflectionClass(Normcore::class))->getEnumUnderlyingType();
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}
