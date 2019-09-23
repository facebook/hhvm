<?hh

class foo {
  public async function __disposeAsync() {
    yield foo();
  }
}

