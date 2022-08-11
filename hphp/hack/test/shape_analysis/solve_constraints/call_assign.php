<?hh

function f(): void {
  g(dict['a' => 42]);
}

function g(dict<string,mixed> $d): void {
  $d['b'] = true;
  inspect($d);
}
