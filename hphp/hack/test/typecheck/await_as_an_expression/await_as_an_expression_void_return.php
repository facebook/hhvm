<?hh // strict

async function genx(): Awaitable<void> {}

async function foo(): Awaitable<void> {
  return await genx();
}
