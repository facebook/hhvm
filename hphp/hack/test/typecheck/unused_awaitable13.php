<?hh // strict

async function f(): Awaitable<int> {
  return 1;
}

function g(): ?Awaitable<int> {
  return null;
}

function h(): void {
  $f = f();
  $g = g();

  $x = $f ?? $g;
}
