<?hh
<<__Rx, __OnlyRxIfArgs>>
function map_with_key<Tk as arraykey, Tv1, Tv2>(
  <<
    __MaybeMutable,
    __OnlyRxIfImpl(\HH\Rx\KeyedTraversable::class)
  >>KeyedTraversable<Tk, Tv1> $traversable,
  <<__OnlyRxIfRxFunc>>(function(Tk, Tv1): Tv2) $value_func,
): dict<Tk, Tv2> {
  $result = dict[];
  return $result;
}

<<__Rx, __OnlyRxIfArgs>>
async function gen_filter_with_key<Tk as arraykey, Tv>(
  KeyedContainer<Tk, Tv> $traversable,
  <<__OnlyRxIfRxFunc>>(function(Tk, Tv): Awaitable<bool>) $predicate,
): Awaitable<dict<Tk, Tv>> {
  $tests = await $traversable
    |> map_with_key(
      $$,
      <<__RxOfScope>> async ($k, $v) ==> await $predicate($k, $v),
    )
    |> gen($$);
  $result = dict[];
  return $result;
}
