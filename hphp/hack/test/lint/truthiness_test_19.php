<?hh

function test(Stringish $x): void {
  if ($x) {
    do_something();
  }
}

function do_something(): void {}
