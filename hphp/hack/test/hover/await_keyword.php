<?hh

async function bar(): Awaitable<void> {}

async function foo(): Awaitable<void> {
  await bar();
  // ^ hover-at-caret
}
