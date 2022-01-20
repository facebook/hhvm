<?hh

class Foo {
  public function foo(): void {
    $g = function($this) {
      $this->foo();
    };
  }
}
