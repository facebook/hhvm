<?hh

function f(dict<string, mixed> $d): void {
  idx($d, 'b');
}

function main(): void {
  f(dict['a' => 42]);
}
