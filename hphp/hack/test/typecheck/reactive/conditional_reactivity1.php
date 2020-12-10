<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>
<<__Rx, __AtMostRxAsArgs>>
function map_with_key<Tk as arraykey, Tv1, Tv2>(
  <<
    __MaybeMutable,
    __OnlyRxIfImpl(\HH\Rx\KeyedTraversable::class)
  >>KeyedTraversable<Tk, Tv1> $traversable,
  <<__AtMostRxAsFunc>>(function(Tk, Tv1): Tv2) $value_func,
): dict<Tk, Tv2> {
  $result = dict[];
  return $result;
}

<<__Rx, __AtMostRxAsArgs>>
async function gen_filter_with_key<Tk as arraykey, Tv>(
  KeyedContainer<Tk, Tv> $traversable,
  <<__AtMostRxAsFunc>>(function(Tk, Tv): Awaitable<bool>) $predicate,
): Awaitable<dict<Tk, Tv>> {
  $tests = await ($traversable
    |> map_with_key(
      $$,
      async ($k, $v) ==> await $predicate($k, $v),
    )
    |> gen($$));
  $result = dict[];
  return $result;
}

<<__Rx>>
async function gen(nonnull $_): Awaitable<nonnull> { return 42; }
