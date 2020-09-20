<?hh // strict

class NonAwaitable<T> {}

async function f(): Awaitable<int> {
  return 1;
}

function g(): void {
  $f = f();
  if ($f is NonAwaitable<_>) {
  }
}
