<?hh

async function f() {
  yield 1;
}

async function g(): Awaitable<void> {
  foreach (f() await as $x) {
  }
}
