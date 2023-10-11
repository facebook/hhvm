<?hh

class Bar {}

class Foo {
  <<__Memoize>>
  public async function someMethod(Bar $arg): Awaitable<string> {
    return "hello";
  }
}
