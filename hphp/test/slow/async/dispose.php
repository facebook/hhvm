<?hh

class foo {
  public async function __dispose() :Awaitable<mixed>{
    await foo();
  }
}

