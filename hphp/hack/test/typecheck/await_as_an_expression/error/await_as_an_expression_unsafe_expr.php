<?hh // strict

async function bar(): Awaitable<void> {}

async function foo(): Awaitable<void> {
  /* UNSAFE_EXPR */ await bar(42);
}
