<?hh // strict

class Bar implements IMemoizeParam {
  public function getInstanceKey(): string {
    return "dummy";
  }
}

class Foo {
  <<__Memoize>>
  public async function someMethod(array<Bar> $arg): Awaitable<string> {
    return "hello";
  }
}

<<__Memoize>>
async function some_async_function(array<Bar> $arg): Awaitable<string> {
  return 'goodbye';
}

<<__Memoize>>
function some_function(array<Bar> $arg): void {}
