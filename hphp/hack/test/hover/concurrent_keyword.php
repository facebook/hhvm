<?hh

async function bar(): Awaitable<void> {}

async function foo(): Awaitable<void> {
  concurrent {
    // ^ hover-at-caret
    await bar();
    await bar();
  }
}
