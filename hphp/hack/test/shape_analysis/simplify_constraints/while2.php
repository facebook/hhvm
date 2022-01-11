<?hh

function f(): void {
  $d = dict['a' => 42];
  $b = false;
  while ($b) {
    $d = dict['b' => 'h'];
    $d['c'] = true;
    inspect($d);
  }
}

function inspect(mixed $_): void {}
