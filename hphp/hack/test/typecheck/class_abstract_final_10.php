<?hh

abstract final class Foo {
  public static function bar() {}
}

class C {
  private ?Foo $member;
}
