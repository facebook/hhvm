<?hh // strict

async function genValue(): Awaitable<int> {
  return 1;
}

async function f(bool $x): Awaitable<int> {
  return await ($x ? genValue() : genValue());
}
