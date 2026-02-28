<?hh
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
interface I {
  public function boo(Handle $h):void;
  public function goo(): Handle;
}

async function unmarkedAsyncReturnDisposable(): Awaitable<Handle> {
  return new Handle();
}

class MyList<T> { }
function badEmbeddedHandle(): MyList<Handle> {
  return new MyList();
}
async function asyncBadEmbeddedHandle(): Awaitable<?Handle> {
  return null;
}

function test():void {
  $foo = function(): Handle {
    // don't even need HH FIXME
    return new Handle();
  };

  // and this does
  $foo = async function(): Awaitable<Handle> {
    // don't even need HH FIXME
    return new Handle();
  };

  // and this does
  $foo = () ==> {
    // don't even need HH FIXME
    return new Handle();
  };

  // and this does
  $foo = async () ==> {
    // don't even need HH FIXME
    return new Handle();
  };
}
