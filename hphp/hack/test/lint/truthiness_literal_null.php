<?hh

function test(): void {
  $x = null;
  if ($x) {
    do_something();
  }
}

function do_something(): void {}
