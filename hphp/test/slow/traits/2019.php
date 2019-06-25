<?hh

trait TestTrait {
  public static function test() {
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
