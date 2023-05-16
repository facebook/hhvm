<?hh

class Klass {
  // Don't suggest a "flip around comma" refactor for params where it's hard to calculate positions
  // Here it's hard to determine a position for "inout".
  public function foo(int $a, /*range-start*//*range-end*/inout int $b): void {}
}
