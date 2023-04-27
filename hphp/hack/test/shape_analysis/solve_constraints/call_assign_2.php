<?hh

function f(): void {
  $d = dict['a' => 42];
  g($d);
  inspect($d);
}

function g(dict<string,mixed> $d): void {
  $d['b'] = true;
}
