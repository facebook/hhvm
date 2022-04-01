<?hh

async function gena<Tk  as arraykey, Tv >(
  KeyedTraversable<Tk, Awaitable<Tv>> $awaitables,
): Awaitable<dict<Tk, Tv>> {
  return dict[];
}

function my_map<Tk  as arraykey, Tv1 , Tv2 >(
  KeyedTraversable<Tk, Tv1> $traversable,
  (function(Tv1): Tv2) $value_func,
): dict<Tk, Tv2>
  { return dict []; }

async function f(mixed $m) : Awaitable<~int> { return 0; }

async function test(
  dict<arraykey, mixed> $app_uids,
): Awaitable<~vec_or_dict<int>> {
  $a = my_map($app_uids, f<>);
  $user_ids = await gena($a);
  return $user_ids;
}
