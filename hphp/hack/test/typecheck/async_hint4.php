<?hh

class C {}

class Foo {
  // Testing method
  public async function wrong_hint(): C {
    throw new Exception();
  }
}
