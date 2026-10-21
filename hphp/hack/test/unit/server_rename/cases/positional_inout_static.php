<?hh
//
class C {
  public static function original(inout int $x): void {
    $x += 1;
  }
}
