<?hh

class C0 {
  <<__NeedsConcrete>>
  public static function m2(): void {
    static::m3();
  }
  <<__NeedsConcrete>>
  public static function m3(): void {}
}

class C1 extends C0 {
  public static function m1(): void {
    // This method should be marked __NeedsConcrete,
    // since `parent` forwards the referent of `static`
    parent::m2();
  }
}

abstract class C2 extends C1 {
  public static function m0(): void {
    static::m1();
  }
  <<__NeedsConcrete, __Override>>
  public static function m3(): void {
    static::abs();
  }
  public static abstract function abs(): void;
}

<<__EntryPoint>>
function main(): void {
  // Fatal error: Cannot call abstract method D::abs()
  // The call graph is m0 -> m1 -> m2 -> m3 -> abs
  C2::m0();
}
