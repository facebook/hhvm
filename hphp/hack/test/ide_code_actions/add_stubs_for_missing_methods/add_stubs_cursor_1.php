<?hh

interface TheInterface {
  public function someMethod(): void;
}

class C implements TheInterface {
                  // ^ at-caret
  public function bar(): void {
    3 + true;
  }
}
