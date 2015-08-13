<?hh // strict

class Bar {}

class Foo {
  <<__Memoize>>
  public async function someMethod(array<Bar> $arg): Awaitable<string> {
    return "hello";
  }
}
