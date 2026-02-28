<?hh

function foo(XHPChild $x): void {
  if ($x) {
    do_something();
  }
}

function do_something(): void {}
