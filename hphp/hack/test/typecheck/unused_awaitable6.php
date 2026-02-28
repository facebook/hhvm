<?hh

async function f(): Awaitable<int> {
  return 1;
}

function g(): void {
  $f = f();
  if (!$f) {
  }
}
