<?hh // strict

function f(inout Vector<int> $v): void {
  $v = Vector {};
}

function test(): void {
  $x = Map { 'wow' => Vector { 256 } };
  f(inout $x['nope']);
}
