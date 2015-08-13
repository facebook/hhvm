<?hh // strict

class Bar implements IMemoizeParam {
  public function getInstanceKey(): string {
    return "dummy";
  }
}

class BarDerived extends Bar {}

class Foo {
  <<__Memoize>>
  public async function someMethod(BarDerived $arg): Awaitable<string> {
    return "hello";
  }
}
