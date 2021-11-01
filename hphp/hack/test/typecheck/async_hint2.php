<?hh

class Foo {
  // Testing method
  public async function wrong_hint(): int {
    throw new Exception();
  }
}
