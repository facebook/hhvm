<?hh

function i(?int $x): void {}

function f(): Generator<bool, string, int> {
  $x = yield true => 'troo';
  i($x);
}

function g(): void {
  $gen = f();
  $gen->send('blah');
}
