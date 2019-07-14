<?hh

class foo {
  public async function __disposeAsync() {
    yield foo();
  }
}

<<__EntryPoint>> function main(): void {}
