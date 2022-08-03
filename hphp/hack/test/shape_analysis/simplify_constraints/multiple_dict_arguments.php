<?hh

function f(): void {
  $d = dict['a' => 42];
  $e = dict['b' => true];
  inspect($d);
  inspect($e);
}

function inspect2(mixed $_, mixed $_): void {}
