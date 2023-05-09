<?hh

class Klass {
  // should be no "flip around comma" refactor suggested
  public function foo(int $a,/*range-start*//*range-end*/): void {}
}
