<?hh // strict
class PU {
  enum E {
    case string name;
    :@A (name = "A");
  }

  public static function dname<TF as this:@E>(TF $x): string {
    return static:@E::name($x);
  }

  public static function dname2<TF as this:@E>(TF $x): string {
    return static::dname($x);
  }

  public static function rname(this:@E $x): string {
    return static::dname($x);
  }

  public static function main(): void {
    assert(static::dname2(:@A) == static::rname(:@A));
  }
}
