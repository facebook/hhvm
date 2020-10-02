<?hh // strict

class Bar implements IMemoizeParam {
  public function getInstanceKey(): string {
    return "dummy";
  }
}

class Foo {
  <<__Memoize>>
  public async function someMethod(varray<Bar> $arg): Awaitable<string> {
    return "hello";
  }
}

<<__Memoize>>
async function some_async_function(varray<Bar> $arg): Awaitable<string> {
  return 'goodbye';
}

<<__Memoize>>
function some_function(varray<Bar> $arg): void {}
