<?hh

trait TestTrait {
  public static function test() :mixed{
    return static::class;
  }
}

class A {
  use TestTrait;
}

class B extends A {}
<<__EntryPoint>> function main(): void {
echo B::test();
}
