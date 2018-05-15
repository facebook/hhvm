<?hh

class Foo {
  static async function __callStatic($a, $b) {
    await foo();
  }
}
