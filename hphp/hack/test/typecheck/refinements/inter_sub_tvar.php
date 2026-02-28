<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

async function f(bool $b, Traversable<mixed> $vals): Awaitable<keyset<string>> {
  $strs = await Vec_gen_map(
    $vals,
    async $v ==> {
      if (!($v is string)) {
        return null;
      }
      return $b ? $v : null;
    },
  );

  return Keyset_filter_nulls($strs);
}

function Keyset_filter_nulls<Tv as arraykey>(
  Traversable<?Tv> $traversable,
): keyset<Tv> {
  $result = keyset[];
  foreach ($traversable as $value) {
    if ($value !== null) {
      $result[] = $value;
    }
  }
  return $result;
}

async function Vec_gen_map<Tv1, Tv2>(
  Traversable<Tv1> $traversable,
  (function(Tv1): Awaitable<Tv2>) $async_func,
): Awaitable<vec<Tv2>> {
  return vec[];
}
