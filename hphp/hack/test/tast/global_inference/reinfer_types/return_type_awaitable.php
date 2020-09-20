<?hh // partial

async function foo(int $x): Awaitable<mixed> {
  return $x;
}

class C {
  public async function foo(int $x): Awaitable<mixed> {
    return $x;
  }
}
