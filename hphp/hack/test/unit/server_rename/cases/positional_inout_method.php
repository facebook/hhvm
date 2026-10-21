<?hh
class C {
  public function original(inout int $x): void {
    $x += 1;
  }
}
