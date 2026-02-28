<?hh

class Klass {
  public function foo(): void {
    $x = /*range-start*/1 * 2/*range-end*/;
  }
}
