<?hh // partial
<<__Rx, __AtMostRxAsArgs>>
async function gen_from_keys<Tk as arraykey, Tv>(
  <<__MaybeMutable, __OnlyRxIfImpl(\HH\Rx\Traversable::class)>>Traversable<Tk>
    $keys,
  <<__AtMostRxAsFunc>>(function(Tk): Awaitable<Tv>) $async_func,
): Awaitable<void> {
  nonreactive();
}

function nonreactive(): void {
}
