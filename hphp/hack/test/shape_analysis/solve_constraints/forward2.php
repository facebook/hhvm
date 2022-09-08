<?hh

function f(dict<string, mixed> $d): void {
  idx($d, 'b');
}

function g(): void {
  f(dict['a' => 42]);
}

function h(): void {
  f(dict['a' => true]);
}
