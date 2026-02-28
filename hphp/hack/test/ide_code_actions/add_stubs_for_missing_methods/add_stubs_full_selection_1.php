<?hh

interface TheInterface {
  public function someMethod(): void;
}

class C implements /*range-start*/TheInterface/*range-end*/ {
  public function bar(): void {
    3 + true;
  }
}
