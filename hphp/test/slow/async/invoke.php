<?hh

class Foo {
  async function __invoke(...$args) :Awaitable<mixed>{
    await foo();
  }
}

