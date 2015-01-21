<?hh

abstract class C1 {
  abstract const int X;
  public function f(): string {
    $x = self::X;
    return $x;
  }
}
