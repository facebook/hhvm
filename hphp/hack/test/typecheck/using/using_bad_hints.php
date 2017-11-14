<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Handle implements IDisposable {
  public function __dispose(): void {}
  public function foo(): void {}
}
class AsyncHandle implements IAsyncDisposable {
  public async function __disposeAsync(): Awaitable<void> {}
  public function bar(): void {}
}

function bad1(Handle $h): void {}
function bad2(IAsyncDisposable $x): void {}
class C {
  public function bad3(AsyncHandle $h): void {}
}
