<?hh

class foo {
  public async function __disposeAsync() {
    await foo();
  }
}

var_dump(new foo());
