<?hh

class Foo {
  async function __invoke(...$args) {
    await foo();
  }
}

