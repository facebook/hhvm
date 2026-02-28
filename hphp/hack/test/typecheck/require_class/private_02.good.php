<?hh

final class MyFoo {
  use MyTrait;

  private static function foo(): void { self::bar(); }
}

trait MyTrait {
  require class MyFoo;

  private static function bar(): void {}
}
