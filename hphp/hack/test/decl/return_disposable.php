<?hh // strict

final class Bar implements IDisposable {
  public function __dispose(): void {}
}

<<__ReturnDisposable>>
async function gen(): Awaitable<Bar> {
  return new Bar();
}
