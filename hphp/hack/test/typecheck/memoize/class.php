<?hh

class Bar implements IMemoizeParam {
  public function getInstanceKey(): string {
    return "dummy";
  }
}

class Foo {
  <<__Memoize>>
  public async function someMethod(Bar $arg): Awaitable<string> {
    return "hello";
  }
}

<<__Memoize>>
async function some_async_function(Bar $arg): Awaitable<void> {}

<<__Memoize>>
function some_function(Bar $arg): string {
  return 'goodbye';
}
