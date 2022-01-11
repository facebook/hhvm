<?hh

function f(): void {
  $d = dict['a' => 42];
  $e = dict['b' => true];
  inspect($d, $e);
}

function inspect(mixed $_, mixed $_): void {}
