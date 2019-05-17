<?hh

class Foo {
  async function __invoke(...$args) {
    await foo();
  }
}

<<__EntryPoint>> function main(): void {}
