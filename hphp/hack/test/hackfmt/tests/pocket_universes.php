<?hh // strict

class C {
  enum Field {
    case type T0;
    case type T1;
    case string name;
    case T0 default_value;
    case T1 stuff;

    :@A (
      type T0 = int,
      type T1 = string,
      name = "A",
      default_value = 42,
      stuff = "stuff" );

    :@B (
      type T0 = string,
      type T1 = int,
      name = "B",
      default_value = "default_value",
      stuff = 1664 );
  }

  enum X {
    :@C;
  }

  public static function get_name(this:@Field $x): string {
    return static:@Field::name($x);
  }

  public static function get_default<TF as this:@Field>(TF $x): TF:@T0 {
    return static:@Field::default_value($x);
  }

  public static function a_default(): int {
    return static::get_default(:@A);
  }

  public static function b_default(): string {
    return static::get_default(:@B);
  }
}
