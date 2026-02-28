<?hh

async function f(): Awaitable<int> {
  return 1;
}

async function g(): Awaitable<?int> {
  return null;
}

function h(): void {
  $f = f();
  $g = g();

  $x = $f && $g;
  if ($x) {
  }
}
