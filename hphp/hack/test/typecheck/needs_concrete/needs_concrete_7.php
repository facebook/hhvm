<?hh

abstract class C {
  <<__NeedsConcrete>>
  public static function foo(): void {
    static::bar();
  }
  public abstract static function bar(): void;
}

final class D extends C {
  public static function bar(): void {
    echo "D::bar\n";
  }
  <<__NeedsConcrete>>
  public static function testparentcall(): void {
    parent::foo();
  }
}
