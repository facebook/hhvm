<?hh

abstract final class C2 {
  public static function m(): void {
    new static(); // error
  }
}
