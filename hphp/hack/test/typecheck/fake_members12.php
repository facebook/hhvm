<?hh

function bar(): void {}

function baz(int $x): void {}

class Foo {
  public ?int $x;

  public function f(bool $y): void {
    if ($this->x === null) {
      return;
    }
    $y ? bar() : null;
    // function call bar() invalidates info about $this->x
    baz($this->x);
  }
}
