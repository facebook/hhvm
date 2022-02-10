<?hh // strict

class Foo {}

function test(Foo $x): void {
  if ($x) {
    do_something();
  }
}

function do_something(): void {}
