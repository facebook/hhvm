<?hh
// Copyright 2004-present Facebook. All Rights Reserved.


function C_reduce<Tv, Ta>(
  Traversable<Tv> $traversable,
  (function(Ta, Tv): Ta) $accumulator,
  Ta $initial,
): Ta {
  return $initial;
}

function C_contains_key<Tk as arraykey, Tv>(
  KeyedContainer<Tk, Tv> $container,
  Tk $key,
): bool {
  return false;
}

function testit(vec<string> $arg): dict<string,vec<string>> {
  $const_map = C_reduce(
      $arg,
      ($accumulator, $val) ==> {
        $b = C_contains_key($accumulator, $val);
        $accumulator[$val] = vec[$val];
        return $accumulator;
      },
      dict[],
    );
  return $const_map;
}
