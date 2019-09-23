<?hh

class foo {
  public async function __dispose() {
    await foo();
  }
}

