<?hh // strict

class Implementor implements Awaitable<int> {
  public function getWaitHandle(): WaitHandle<int> {
    return f()->getWaitHandle();
  }
}

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

  if ($f instanceof Awaitable) {
  }
  if ($g instanceof Implementor) {
  }
  if ($h === null) {
  }
  $x = $h ?? $f;
}
