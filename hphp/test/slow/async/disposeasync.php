<?hh

class foo {
  public async function __disposeAsync() {
    await foo();
  }
}


<<__EntryPoint>>
function main_disposeasync() {
var_dump(new foo());
}
