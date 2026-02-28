<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

async function foo(mixed $a, mixed $b, mixed $c, mixed $x) : Awaitable<void> {
  await $a++;  // still await ($a++) to force people to make their
               //  callsites (await $a)++ to clarify readability.
  await ++$a;  // and this doesn't care because they're unary prefix.
  await $a + $x;
  await await $a;
  await $a->bar($x);
  await $a->bar($x) is vec;
  await $a * $x;
  await $a |> $b($$) |> $c($$);
  await $a ? $b : $c;
}
