<?hh // strict
// OK: refer to T inside mapping expression
class PU13 {
  public static function id<T>(T $x): T { return $x; }

  enum X {
    case type T;
    case int v;
    :@X (type T = int, v = static::id<T>(1));
  }
}
