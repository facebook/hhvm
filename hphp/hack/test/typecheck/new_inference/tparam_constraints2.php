<?hh

function keyset_gen_map<Tv, Tk as arraykey>(
  Traversable<Tv> $_,
  (function(Tv): Awaitable<Tk>) $_,
): Awaitable<keyset<Tk>> {

}

async function f(int $_): Awaitable<void> {}

async function test(keyset<int> $xs): Awaitable<void> {
  await keyset_gen_map(
    $xs,
    async (int $x) ==> await f($x),
  );
}
