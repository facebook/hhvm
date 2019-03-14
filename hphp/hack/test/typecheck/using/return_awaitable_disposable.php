<?hh // partial

class Foo implements IDisposable {
  public function __dispose(): void {
  }
}

<<__ReturnDisposable>>
async function gen_foo(): Awaitable<IDisposable> {
  return new Foo();
}

<<__ReturnDisposable>>
async function gen_bar(): Awaitable<IDisposable> {
  return await gen_foo();
}
