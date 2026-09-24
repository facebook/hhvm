<?hh

// There should be no errors when needs_concrete=false

abstract class C1 {
  public static abstract function abs(): void;
  public static function fooz(): void {
    // When the needs-concrete checks are enabled, the next line is an error.
    static::abs();
  }
}

abstract class C2 extends C1 {
  // When the needs-concrete checks are enabled, the next line is an error.
  // for a bad override: __NeedsConcrete cannot override non-__NeedsConcrete
  <<__NeedsConcrete, __Override>>
  public static function fooz(): void {}
}
