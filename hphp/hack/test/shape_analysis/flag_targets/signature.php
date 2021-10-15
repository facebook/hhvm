<?hh

class C {
  public function f(
    dict<int, mixed> $_,
    darray<string, int> $_,
  ): dict<arraykey,mixed> {
    return dict[];
  }
}

async function f(): Awaitable<dict<string,mixed>> {
  return dict[];
}
