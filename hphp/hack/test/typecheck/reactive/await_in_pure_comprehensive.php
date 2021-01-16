<?hh
<<__Pure>>
async function gen_int(int $x = 0)[]: Awaitable<int> { return 0; }

<<__Pure>>
async function gen_maybe_int(int $x = 0)[]: Awaitable<?int> { return $x; }

<<__Pure>>
async function maybe_gen_int(int $x = 0)[]: ?Awaitable<int> {
  return gen_int();
}

<<__Pure>>
function takes_ints(int $_, int $_)[]: void {}

<<__Pure>>
async function gen_awaitable(Awaitable<int> $x)[]: Awaitable<int> {
  return await $x;
}

<<__Pure>>
async function gen_awaitables(
  Awaitable<int> $_, Awaitable<int> $_
)[]: Awaitable<void> {}

<<__Pure>>
async function test()[]: Awaitable<int> {
  $ok_call = await gen_int();
  await (true ? gen_int() : gen_int());

  $broken = (true ? gen_int() : gen_int());  // BUG: should be error
  $broken2 = ((await gen_int()) ?? gen_int()); // BUG: should be error

  await gen_awaitables(
    (false ? gen_int() : gen_int()), // BUG: should be error
    $broken, // BUG?: should be error
  );

  $broken3 = gen_awaitables( // error as expected
    // NOTE: interested in RHS of ??
    maybe_gen_int() ?? gen_int(), // BUG: should be error
    (true ? gen_int() : gen_int()),
  );

  await gen_awaitables(
    gen_awaitable(gen_int()), // error as expected
    gen_int(), // error as expected
  );

  takes_ints( // ok (both immediately awaited)
    (await gen_maybe_int()) ?? 42,
    await (true ? gen_int() : gen_int()),
  );

  $ok_pipe = await (1 |> gen_int($$)); // ok
  await (gen_int() |> gen_awaitable($$)); // ok

  await (gen_int() |> gen_awaitable(gen_int())); // error (first gen_int() unawaited)

  await (gen_int() |> gen_awaitables(
    $$, // gen_int() // error as expected
    $$,
  )); // ok

  return 0;
}
