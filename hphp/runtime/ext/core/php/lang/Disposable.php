<?hh

interface IDisposable {
  public function __dispose(): void;
}

interface IAsyncDisposable {
  public function __disposeAsync(): Awaitable<void>;
}
