<?hh

abstract final class Foo {
  public static function bar(): void {}
}

class C {
  private static ?Foo $member;
}
