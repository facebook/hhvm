<?hh // strict

async function bar(): Awaitable<void> {}

async function foo(): Awaitable<void> {
  await /* UNSAFE_EXPR */ bar(42);
}
