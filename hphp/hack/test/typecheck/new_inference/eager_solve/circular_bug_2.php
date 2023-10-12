<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

interface I {
  public function query(): Awaitable<darray<int,string>>;
}

async function genEnforce(
  arraykey $key,
): Awaitable<I> {
  throw new Exception();
}

async function genmk<Tk as arraykey, Tv>(
  (function (Tk): Awaitable<Tv>) $gen,
  Traversable<Tk> $keys,
): Awaitable<Map<Tk, Tv>> {
  return Map{};
}
function keys<Tk, Tv>(
  KeyedTraversable<Tk, Tv> $traversable,
): vec<Tk> {
  return vec[];
}
async function stuff(varray<int> $group_ids): Awaitable<void> {
  $groups = await genmk(
    async $id ==> await genEnforce($id),
    $group_ids,
  );
  $members = await genmk(
    async $id ==> await $groups[$id]->query(),
    keys($groups),
  );
}
