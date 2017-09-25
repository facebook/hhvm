<?hh // strict

function byVal(int $x): void {
  echo 'byVal is called, $x is: ';
  var_dump($x);
}

function test(): void {
  $x = null;
  byVal($x?->y); // ok
}

test();
