<?hh

class C {
  public static function f(vec<int> $foo): void {
    nameof $foo[3];
  }
}
