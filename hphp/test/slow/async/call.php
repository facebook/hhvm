<?hh

class Foo {
  async function __call($a, $b) {
    await foo();
  }
}

<<__EntryPoint>> function main(): void {}
