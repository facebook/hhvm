<?hh

class foo {
  public async function __disposeAsync() :AsyncGenerator<mixed,mixed,void>{
    yield foo();
  }
}

