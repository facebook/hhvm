<?hh

function bar() {}

function baz(int $x) {}

class Foo {
  public ?int $x;

  public function f($y): void {
    if ($this->x === null) {
      return;
    }
    $y ? bar() : null;
    // function call bar() invalidates info about $this->x
    baz($this->x);
  }
}
