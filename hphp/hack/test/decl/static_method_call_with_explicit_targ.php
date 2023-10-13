<?hh

class C {
  public static function f(): int {
    if (static::equal<int>(0, 0)) {
      return 1;
    }
    return 0;
  }

  public static function equal<T>(T $a, T $b): bool {
    return $a === $b;
  }
}
