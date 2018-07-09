<?hh // strict

<<__Rx>>
async function genValue(): Awaitable<int> {
  return 1;
}
<<__Rx>>
async function f(bool $x): Awaitable<int> {
  return await ($x ? genValue() : genValue());
}
