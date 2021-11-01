<?hh

function i(?int $x): void {}

function f(): Generator<bool, string, string> {
  $x = yield true => 'troo';
  i($x);
}

function g(): void {
  $gen = f();
  $gen->send(1);
}
