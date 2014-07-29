<?hh

function i(?bool $x): void {}

function f(): Generator<bool, string, int> {
  $x = yield true => 'troo';
  i($x);
}

function g(): void {
  $gen = f();
  $gen->send(1);
}
