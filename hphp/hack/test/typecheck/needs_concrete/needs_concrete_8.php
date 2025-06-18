<?hh

abstract class C {
  public static abstract function abs(): void;
  <<__NeedsConcrete>>
  public static function m(): void {
    static::abs();
  }
}

final class D extends C {
  public static function abs(): void {}
  // __NeedsConcrete is not required here, since we know that all abstract members are implemented in final classes
  public static function m(): void {
    parent::m();
  }
}

<<__EntryPoint>>
function main(): void {
  D::m(); // OK
}
