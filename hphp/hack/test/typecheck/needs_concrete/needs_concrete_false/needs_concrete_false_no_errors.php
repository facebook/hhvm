<?hh

// There should be no errors when needs_concrete=false

abstract class C1 {
  public static abstract function abs(): void;
  public static function fooz(): void {
    // when needs_concrete=true there's an error on the next line
    static::abs();
  }
}

abstract class C2 extends C1 {
  // when needs_concrete=true there's an error on the next line
  // for a bad override: __NeedsConcrete cannot override non-__NeedsConcrete
  <<__NeedsConcrete, __Override>>
  public static function fooz(): void {}
}
