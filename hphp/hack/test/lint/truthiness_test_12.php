<?hh

class Foo {}

function test(bool $b, Foo $f1, ?Foo $f2): void {
  $x = $b ? $f1 : $f2;
  if ($x) {
    do_something();
  }
}

function do_something(): void {}
