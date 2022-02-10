<?hh // strict

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

function test(bool $b, Bar $bar): void {
  $x = $b ? $bar : $b;
  // $x : (Bar | bool). Even though Bar is always-truthy, we permit this check
  // because bool is possibly-falsy.
  if ($x) {
    do_something();
  }
}

function do_something(): void {}
