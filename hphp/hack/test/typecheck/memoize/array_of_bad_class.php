<?hh // strict

class Bar {}

class Foo {
  <<__Memoize>>
  public async function someMethod(varray<Bar> $arg): Awaitable<string> {
    return "hello";
  }
}
