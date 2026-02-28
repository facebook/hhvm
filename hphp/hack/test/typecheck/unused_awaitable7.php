<?hh

async function f(): Awaitable<int> {
  return 1;
}

async function g(): Awaitable<?int> {
  return null;
}

function h(): ?Awaitable<int> {
  return null;
}

function i(): void {
  $f = f();
  $g = g();
  $h = h();

  if ($f is Awaitable<_>) {
  }
  if ($g is Awaitable<_>) {
  }
  if ($h === null) {
  }
  $x = $h ?? $f;
  if ($x) {}
}
