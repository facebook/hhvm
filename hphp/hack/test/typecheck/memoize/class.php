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
