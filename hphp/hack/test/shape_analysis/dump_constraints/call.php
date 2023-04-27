<?hh

function f(): void {
  $d1 = dict['a' => 42];
  $d2 = dict['b' => 42];
  g($d1, $d2);
}

function g(dict<string,mixed> $d1, dict<string,mixed> $d2): void {
  h($d1);
}

function h(dict<string,mixed> $d) : void {
  $d['c'] = 42;
}
