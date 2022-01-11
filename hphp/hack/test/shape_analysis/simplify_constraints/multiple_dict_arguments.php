<?hh

function f(): void {
  $d = dict['a' => 42];
  $e = dict['b' => true];
  inspect2($d, $e);
}

function inspect2(mixed $_, mixed $_): void {}
