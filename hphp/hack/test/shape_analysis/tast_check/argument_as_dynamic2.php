<?hh

function g(int $i): void {}

function f(): void {
  $d = dict['a' => 42];
  g(42);
}
