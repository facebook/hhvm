<?hh

class foo {
  public async function __disposeAsync() :Awaitable<mixed>{
    await foo();
  }
}


<<__EntryPoint>>
function main_disposeasync() :mixed{
var_dump(new foo());
}
