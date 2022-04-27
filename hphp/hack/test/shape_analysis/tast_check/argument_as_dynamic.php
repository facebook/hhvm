<?hh

function g(dict<string, mixed> $d): void {}

function f(): void {
  $d = dict['a' => 42];
  $e = dict['b' => true];
  g($d);
}
