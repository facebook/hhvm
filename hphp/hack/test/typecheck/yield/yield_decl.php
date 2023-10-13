<?hh

async function foo() {
  yield 10;
}

function bar(): void {
  // without the correct inference of foo as an async generator, this would
  // raise an error about an Awaitable being used dangerously
  foo();
}
