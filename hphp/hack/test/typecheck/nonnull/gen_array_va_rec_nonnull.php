<?hh // strict

async function f(nonnull $x): Awaitable<void> {
  await gen_array_rec(tuple($x, $x));
}
