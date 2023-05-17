<?hh

class Klass {
  public function foo(): void {
    // bad range, so no code actions expected
    $x = /*range-start*/ 1 * (2/*range-end*/ + 3);
  }
}
