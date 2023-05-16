<?hh

class Klass {
  // should be no "flip around comma" refactor suggested because only one param
  public function foo(int $a,/*range-start*//*range-end*/): void {}
}
