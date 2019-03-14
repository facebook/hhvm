<?hh // partial

function f(): void {
  foreach (g() await as $x) {
  }
}

function g(): Awaitable<int> {
  throw new Exception('unimplemented');
}
