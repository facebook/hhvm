<?hh

class Foo implements Iterator<int> {
  public function current(): int {
    return 0;
  }
  public function next(): void {}
  public function rewind(): void {}
  public function valid(): bool {
    return true;
  }
}

class Bar extends Foo {}

function test(Bar $x): void {
  if ($x) {
    do_something();
  }
}

function do_something(): void {}
