<?hh // strict

function byRef(int &$x): void {}

function test(): void {
  $x = null;
  byRef($x?->y); // error
}

test();
