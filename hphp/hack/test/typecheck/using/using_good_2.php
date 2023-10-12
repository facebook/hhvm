<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Handle implements IDisposable {
  public function __dispose():void { }
}
class AsyncHandle implements IAsyncDisposable {
  public async function __disposeAsync():Awaitable<void> { }
}

async function global_lol(bool $b): Awaitable<string> {
  using (new Handle()) {
    $x = "global_wat";
  }
  return $x;
}

async function global_lol2(): Awaitable<string> {
  using new Handle();
  return "wat";
}

function global_lol3(): string {
  using new Handle();
  return "wat";
}
