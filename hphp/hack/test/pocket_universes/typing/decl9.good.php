<?hh // strict
// OK: refer to T as return type and in mapping
class PU12 {
  public static function id<T>(T $x): T { return $x; }

  enum X {
    case type T;
    case int v;
    :@X (type T = int, v = static::id<int>(1));
  }
}
