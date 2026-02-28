<?hh

async function f(): Awaitable<int> {
  return 1;
}

function g(bool $b): void {
  $f = f();
  if ($b || $f) {
  }
}
