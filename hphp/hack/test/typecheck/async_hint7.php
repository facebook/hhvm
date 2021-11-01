<?hh

class Foo {
  // Testing method
  public async function right_hint(): Awaitable<int> {
    throw new Exception();
  }

  // Testing method
  public async function no_hint() {
    throw new Exception();
  }
}
