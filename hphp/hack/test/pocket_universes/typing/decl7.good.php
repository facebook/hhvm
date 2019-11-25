<?hh // strict
// OK: T as return type too
class PU14 {
  public static function id<T>(T $x): T { return $x; }

  enum X {
    case type T;
    case T v;
    :@X (type T = int, v = static::id<int>(1));
  }
}
