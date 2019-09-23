<?hh

class Foo {
  async function __call($a, $b) {
    await foo();
  }
}

